#include "tariffServerMain.hpp"


tariffServerMain::tariffServerMain() :
    file{std::make_shared<FileManager>()}, 
    tData{std::make_shared<tariffSentenceData>()},
    timer{std::make_shared<ChronoTime>()},
    utils{std::make_shared<Utils>(timer, tData)}, 
    logger{std::make_shared<LogManager>(timer, file)},
    config{std::make_unique<ConfigManager>(logger, utils, file)},
    threadCount{4}, 
    threads{std::make_shared<threadPool>(threadCount, logger)},
    sock{std::make_unique<SocketManager>(logger, epoll)},
    epoll{std::make_shared<EpollManager>()}, 
    db{std::make_unique<databaseManager>(logger, tData)}
{}


tariffServerMain::~tariffServerMain(){}


void tariffServerMain::runServer(std::stop_token stopToken){
    // register stop callback
    std::stop_callback stopCallback(stopToken, [this](){
        logger->debug("Stop callback triggered. Stopping server main loop.");
        serverCv.notify_one();
    });

    // server loop
    try{
        logger->setThreadPool(threads);
        threads->start();
        db->start();
        sock->start();
        // this->startMonitoringThread();
        this->startSqlDbFlushThread();
        // this->timedOutShutdown(3);
        int serverSocket = sock->getServerSocket();
        epoll->addSocket(serverSocket, EPOLLIN | EPOLLET);
        logger->debug("tariffServerMain::runServer(): Server socket added to epoll manager.");
        sock->setOnClientDataReceivedCallback([this](int clientSocket, const std::string_view data) {
            this->receivedMessageHandler(clientSocket, data);
        });
        sock->networkEventLoop();
    }catch(std::exception &ex){
        std::string exMsg = std::format("tariffServerMain::runServer(): {}", ex.what());
        logger->error(exMsg);
        shutdown();
    }catch(...){
        std::string exMsg = "tariffServerMain::runServer(): Unknown error occurred.";
        logger->error(exMsg);
        shutdown();
    }
        
    // Wait for stop signal
    {
        std::unique_lock<std::mutex> lock(serverMtx);
        serverCv.wait(lock, [&stopToken](){
            return stopToken.stop_requested();
        });
    }
}


void tariffServerMain::startSqlDbFlushThread(){
    int dbFlushIntervalMs = 3000;
    sqlDbFlushThread = std::jthread([this, dbFlushIntervalMs](std::stop_token dbFlushSttk){
        while(!dbFlushSttk.stop_requested()){
            std::this_thread::sleep_for(std::chrono::milliseconds(dbFlushIntervalMs));
            if(!dbFlushSttk.stop_requested()){
                threads->threadPoolSubmitTask(workerRole::DATABASE, [this](){
                    db->flushTariffDataBufferToSql();
                });
            }
        }
    });
}


void tariffServerMain::startMonitoringThread(){
    int monitoringIntervalMs = 2000;
    monitoringThread = std::jthread([this, monitoringIntervalMs](std::stop_token monitorSttk){
        while(!monitorSttk.stop_requested()){
            logger->debug(std::format("Active threads: {}, Thread pool size: {}", threads->getActiveThreadCount(), threads->getThreadPoolSize()));
            logger->debug(threads->getThreadPoolQueuesStatistics());
            std::this_thread::sleep_for(std::chrono::milliseconds(monitoringIntervalMs));
        }
    });
}


void tariffServerMain::sendTestTasksToThreadPool(){
    int taskSubmitIntervalMs = 2;
    int taskSubmitDelayMs = 800;
    tasksThread = std::jthread([this, taskSubmitIntervalMs, taskSubmitDelayMs](std::stop_token sttk){
        while(!sttk.stop_requested()){
            if(!threads->getThreadPoolOccupancy(1.0)){
                logger->warning("Thread pool capacity limit reached. Waiting.");
                std::this_thread::sleep_for(std::chrono::milliseconds(taskSubmitDelayMs));
                continue;
            }
            threads->threadPoolSubmitTask(workerRole::COMMON, [](){ return 3 * 3; });
            std::this_thread::sleep_for(std::chrono::milliseconds(taskSubmitIntervalMs));
        }
    });
}


void tariffServerMain::timedOutShutdown(int secondsTimeout){
    auto timedOutShutdownThread = std::thread([this, secondsTimeout](){
        std::this_thread::sleep_for(std::chrono::seconds(secondsTimeout));
        logger->debug("Timed out shutdown triggered. Stopping server.");
        this->stop();
    });
    timedOutShutdownThread.detach();
}


void tariffServerMain::start(){
    this->runServer(serverStopSource.get_token());
}


void tariffServerMain::stop(){
    logger->debug("TariffServer stopping.");
    if(!tariffServerRunningFlag){ return; }
    tariffServerRunningFlag = false;
    sqlDbFlushThread.request_stop();
    serverStopSource.request_stop();
    {
        std::lock_guard<std::mutex> lock(serverMtx);
        serverCv.notify_one();
    }
    db->flushTariffDataBufferToSql();
    sock->stop();
    // monitoringThread.request_stop();
    db->stop();
    threads->stop();
}


void tariffServerMain::shutdown(){
    serverStopSource.request_stop();
}


void tariffServerMain::receivedMessageHandler(int clientSocket, const std::string_view receivedMessage){
    try{
        // processing a tariff sentences bulk
        std::string rawData = std::string(receivedMessage);
        threads->threadPoolSubmitTask(workerRole::COMMON, [this, rawDataBulk = std::move(rawData)]() mutable{

            if(rawDataBulk.empty() || rawDataBulk.size() < 60){ return; }
            std::stringstream bulkStream(std::move(rawDataBulk));
            std::string line;
            while(getline(bulkStream, line)){
                if(!utils->isValidTariffSentence(line)){ continue; }
                
                // parsing a sentence
                threads->threadPoolSubmitTask(workerRole::COMMON, [this, line = std::move(line)]() mutable{
                    try{
                        auto tData = utils->parseTariffSentence(line);
                        // logger->debug("From PARSING thread: " + tData.dateTime);

                        threads->threadPoolSubmitTask(workerRole::DATABASE, [this, tData = std::move(tData)]() mutable{
                            // logger->debug("From DB thread: " + tData.dateTime + " " + tData.calling + " " + tData.dialled + " " + std::to_string(tData.duration) + " " + tData.trunk);
                            db->enqueueTariffData(std::move(tData));
                        });
                    }catch(std::exception &ex){
                        logger->error(std::format("tariffServerMain::receivedMessageHandler(): Parsing error: {}", ex.what()));
                    }
                });
            }
        });
    }catch(std::exception &ex){
        std::string exMsg = std::format("tariffServerMain::receivedMessageHandler(): {}", ex.what());
        std::cerr << exMsg << std::endl;
        logger->error(exMsg);
    }catch(...){
        std::string exMsg = "tariffServerMain::receivedMessageHandler(): Unknown error occurred.";
        std::cerr << exMsg << std::endl;
        logger->error(exMsg);
    }
}


bool tariffServerMain::processArguments(int argc, char **argv){
    try{
        logger->debug("Processing configuration arguments.");
        if(argc < 2){
            logger->error("Arguments: No arguments given. Exiting.");
            return false;
        }
        std::vector<std::string> serverArguments;
        serverArguments.reserve(argc - 1);
        for(int i = 1; i < argc; i++){
            serverArguments.emplace_back(std::string(argv[i]));
        }
        
        static const std::vector<std::string> possibleArguments{"--config", "--version", "--help"};
        for (size_t i = 0; i < serverArguments.size(); i++) {
            if(!tariffServerRunningFlag) { break; }
            const auto& arg = serverArguments[i];
            logger->trace(std::format("ARGV[{}]: {}", i + 1, arg));
            bool recognized = false;
            for (const auto& validArg : possibleArguments) {
                if (arg == validArg || arg.starts_with(validArg + "=")) {
                    logger->trace(std::format("Valid argument found: {}", arg));
                    recognized = true;

                    if (arg == "--version") {
                        logger->trace(std::format("Processing argument: {}", arg));
                        std::cout << getVersion();
                        return false;
                    }
                    if (arg == "--usage" || arg == "--help") {
                        logger->trace(std::format("Processing argument: {}", arg));
                        std::cout << getUsage() << std::endl;
                        return false;
                    }
                    if (arg.starts_with("--config=")) {
                        std::string cliConfigFile;
                        if(utils->split(arg, '=').size() > 1){
                            cliConfigFile = utils->split(arg, '=')[1];
                        }
                        logger->debug(std::format("Processing argument: {}", arg));
                        config->setConfigFile(cliConfigFile);
                        config->loadConfigValues();
                        setConfigFileValues(); // set config parameters into other class instances
                        logger->debug(std::format("Setting config file: {}", cliConfigFile));
                        break;
                    }
                }
            }
            if (!recognized) {
                logger->error(std::format("Unknown argument given: {}. Exiting.", arg));
                return false;
            }
        }
        logger->debug(std::format("Processing configuration arguments finished."));
        return true;
    }catch(std::exception &ex){
        std::string exMsg = std::format("tariffServerMain::processArguments(): {}", ex.what());
        std::cerr << exMsg << std::endl;
        logger->error(exMsg);
        return false;
    }
}


void tariffServerMain::setConfigFileValues(){
    try{
        logger->trace("Getting config map configuration parameters.");
        const auto &configMap = config->getConfigMap();
        auto getString = [](const auto& var, const std::string& key) -> std::string {
            if (std::holds_alternative<std::string>(var.at(key))) {
                return std::get<std::string>(var.at(key));
            }
            return {};
        };
        auto getInt = [](const auto& var, const std::string& key) -> int {
            if (std::holds_alternative<int>(var.at(key))) {
                return std::get<int>(var.at(key));
            }
            return 0;
        };
        auto getBool = [](const auto& var, const std::string& key) -> bool {
            if (std::holds_alternative<bool>(var.at(key))) {
                return std::get<bool>(var.at(key));
            }
            if (std::holds_alternative<std::string>(var.at(key))) {
                const auto& val = std::get<std::string>(var.at(key));
                return (val == "true" || val == "1" || val == "yes");
            }
            return false;
        };

        if(configMap.find("logType") != configMap.end()){
            logger->setLogType(logger->stringToLogType(getString(configMap, "logType")));
            logger->trace(std::format("tariffServerMain::setConfigFileValues(): Service now has set logType: {}.", getString(configMap, "logType")));
        }
        if(configMap.find("logFile") != configMap.end()){
            logger->setLogFile(getString(configMap, "logFile"));
            logger->trace(std::format("tariffServerMain::setConfigFileValues(): Service now has set logFile: {}.", getString(configMap, "logFile")));
        }
        if(configMap.find("logLevel") != configMap.end()){
            logger->setSelectedLogSeverity(getString(configMap, "logLevel"));
            logger->trace(std::format("tariffServerMain::setConfigFileValues(): Service now has set logLevel: {}.", getString(configMap, "logLevel")));
        }
        if(configMap.find("listenPort") != configMap.end()){
            sock->setServerPort(getInt(configMap, "listenPort"));
            logger->trace(std::format("tariffServerMain::setConfigFileValues(): Service now has set listenPort: {}.", getInt(configMap, "listenPort")));
        }
        if(configMap.find("threadCount") != configMap.end()){
            threads->setThreadCount(getInt(configMap, "threadCount"));
            logger->trace(std::format("tariffServerMain::setConfigFileValues(): Service now has set threadCount: {}.", getInt(configMap, "threadCount")));
        }
        if(configMap.find("server") != configMap.end()){
            db->setDatabaseHost(getString(configMap, "server"));
            logger->trace(std::format("tariffServerMain::setConfigFileValues(): Service now has set server: {}.", getString(configMap, "server")));
        }
        if(configMap.find("user") != configMap.end()){
            db->setDatabaseUser(getString(configMap, "user"));
            logger->trace(std::format("tariffServerMain::setConfigFileValues(): Service now has set user: {}.", "************"));
        }
        if(configMap.find("password") != configMap.end()){
            db->setDatabasePassword(getString(configMap, "password"));
            logger->trace(std::format("tariffServerMain::setConfigFileValues(): Service now has set password: {}.", "************"));
        }
        if(configMap.find("database") != configMap.end()){
            db->setDatabaseName(getString(configMap, "database"));
            logger->trace(std::format("tariffServerMain::setConfigFileValues(): Service now has set database: {}.", getString(configMap, "database")));
        }
        if(configMap.find("port") != configMap.end()){
            db->setDatabasePort(getInt(configMap, "port"));
            logger->trace(std::format("tariffServerMain::setConfigFileValues(): Service now has set port: {}.", getInt(configMap, "port")));
        }


    }catch(const std::exception &ex){
        std::string exMsg = std::format("tariffServerMain::setConfigFileValues(): {}", ex.what());
        std::cerr << exMsg << std::endl;
        logger->error(exMsg);
        stop();
    }
}


std::string tariffServerMain::getVersion() const {
    return(
        "\nTariffServer for Avaya systems, version 1.0.10"
        "\nAuthor: Jan Novotny <jan.novotny.cz@gmail.com> 2026 (20260406)"
        "\nProgramming language: C++23"
        "\n\n"
    );
}


std::string tariffServerMain::getUsage() const {
    int spacer = 36;
    return(
        std::format("{} Usage: tariffServer [--config=(path to config file)]\n"
        " Available arguments:\n", getVersion()) +
        std::format("   {:<{}} {}\n", "--help", spacer, "Shows this help and exit") +
        std::format("   {:<{}} {}\n", "--usage", spacer, "Same as help") +
        std::format("   {:<{}} {}\n", "--version", spacer, "Shows version of this tool and exit") +
        std::format("   {:<{}} {}\n", "--config=[\"Path to config file\"]", spacer, "Allows using of configuration file")
    );
}