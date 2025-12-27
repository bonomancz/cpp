#include "../include/httpdServer.hpp"


httpdServer::httpdServer() :
    utils(std::make_shared<Utils>()),
    timer(std::make_shared<ChronoTime>()),
    logger(std::make_shared<LogManager>(timer)),
    threads(std::make_shared<ThreadManager>(logger)),
    sock(std::make_shared<SocketManager>(logger)),
    sslSock(std::make_shared<SslManager>(logger)),
    datas(std::make_shared<DataManager>()),
    config(std::make_shared<ConfigManager>(logger))
{
    logger->debug("Server initialized.");
};


httpdServer::~httpdServer(){ stop(); }


void httpdServer::start(){
    try{
        if (!serverRunningFlag) { return; }
        runServer();
        logger->info("Server started.");
        while(serverRunningFlag){
            logger->debug("Keep-Alive: Server running.");
            try {
				SOCKET clientSocket;
				if ((clientSocket = sock->acceptSock()) != INVALID_SOCKET) {
                    if(sslEnabled){
                        SSL* ssl = sslSock->acceptSsl(clientSocket);
                        threads->submitThreadPoolTask([this, ssl, clientSocket](){
                            this->handleClientSSL(ssl, clientSocket);
                        });
                    }else{
                        threads->submitThreadPoolTask([this, clientSocket](){
                            this->handleClient(sock, clientSocket);
                        });
                    }
                }
			}
			catch (std::runtime_error& ex) {
                std::string exMsg = std::format("httpdServer::start(): {}", ex.what());
                std::cerr << exMsg << std::endl;
                logger->error(exMsg);
			}
        }
    }catch(std::runtime_error &ex){
        std::string exMsg = std::format("httpdServer::start(): {}", ex.what());
        std::cerr << exMsg << std::endl;
        logger->error(exMsg);
    }
}


void httpdServer::runServer(){
    try{
        logger->start();
        logger->debug("Starting WinSock2 API.");
        sock->initWSA();
        logger->debug("Starting ConfigManager.");
        config->loadConfigValues();
        this->getServerConfigParameters();
        logger->debug("Starting SocketManager.");
        sock->build();
        logger->trace("Network socket initialized.");
        if(sslEnabled){
            logger->debug("Starting SSL context.");
            sslSock->initializeContextSSL("./serverCert.pem", "./privateKey.key");
        }
        logger->debug("Starting ThreadManager.");
        threads->start();
        logger->debug("Starting server.");
    }catch(std::runtime_error &ex){
        std::cerr << "httpdServer::runServer(): " << ex.what() << std::endl;        
    }
}


void httpdServer::handleClient(std::shared_ptr<SocketManager> sock, SOCKET clientSocket) {
	try {
		std::string clientMessage;
		auto callbackHandler = [this](SOCKET &clientSocket, std::string &clientMessage) {
			this->receivedMessageHandler(clientSocket, clientMessage);
		};
		sock->receiveSock(clientSocket, clientMessage, callbackHandler);
		closesocket(clientSocket);
	}
	catch(std::runtime_error &ex){
        std::string exMsg = std::format("httpdServer::handleClient(): {}", ex.what());
        std::cerr << exMsg << std::endl;
        logger->error(exMsg);
    }
	catch (...) {
        std::string exMsg = "httpdServer::handleClient(): Unknown error occured.";
        std::cerr << exMsg << std::endl;
        logger->error(exMsg);
	}
}


void httpdServer::handleClientSSL(SSL* ssl, SOCKET clientSocket){
    try {
        std::string clientMessage;
        auto callbackHandler = [this](SSL* ssl, std::string& clientMessage) {
            this->receivedMessageHandlerSSL(ssl, clientMessage);
        };
        sslSock->receiveSockSSL(ssl, clientMessage, callbackHandler);
        sslSock->cleanupSockSSL(ssl);
        closesocket(clientSocket);
    }
    catch (const std::exception& ex) {
        std::string exMsg = std::format("httpdServer::handleClientSSL(): {}", ex.what());
        logger->error(exMsg);
        std::cerr << exMsg << std::endl;
    }
}


void httpdServer::receivedMessageHandler(SOCKET &clientSocket, std::string &clientMessage) {
	try {
		std::string clientInfo = "Client: " + std::string(sock->getClientIpAddr(clientSocket));
		if (datas->isHttpRequest(clientMessage)) {
			std::string sendMessage = datas->getHttpReponse(404);
			if (datas->isExistingUrl(clientMessage)) {
				sendMessage = datas->getHttpReponse(200);
			}
			sock->sendSock(clientSocket, sendMessage);
			clientInfo += ", " + datas->getClientRequest(clientMessage);
			logger->info(clientInfo);
		}
	}
	catch(std::runtime_error &ex){
        std::string exMsg = std::format("httpdServer::receivedMessageHandler(): {}", ex.what());
        std::cerr << exMsg << std::endl;
        logger->error(exMsg);
    }
	catch(...) {
        std::string exMsg = "httpdServer::receivedMessageHandler(): Unknown error occured.";
        std::cerr << exMsg << std::endl;
        logger->error(exMsg);
	}
}


void httpdServer::receivedMessageHandlerSSL(SSL* ssl, std::string& clientMessage) {
    try {
        if (datas->isHttpRequest(clientMessage)) {
            std::string sendMessage = datas->getHttpReponse(404);
            if (datas->isExistingUrl(clientMessage)) {
                sendMessage = datas->getHttpReponse(200);
            }
            sslSock->sendSockSSL(ssl, sendMessage);
            logger->debug("HTTPS request processed successfully.");
        }
    }
    catch (const std::exception& ex) {
        std::string exMsg = std::format("httpdServer::receivedMessageHandlerSSL(): {}", ex.what());
        logger->error(exMsg);
        std::cerr << exMsg << std::endl;
    }
}


void httpdServer::stop(){
    serverRunningFlag = false;
    logger->info("Server stopping.");
    threads->stop();
    logger->info("ThreadManager stopping.");
    logger->stop();
    logger->info("LogManager stopping.");
}


void httpdServer::getServerConfigParameters(){
    try{
        logger->trace(std::format("Setting server hostname: {}.", sock->getServerHostname()));
        config->setHostName(sock->getServerHostname());
        logger->trace("Getting server config map configuration parameters.");
        auto configMap = config->getConfigMap();
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

        logger->trace(std::format("Setting new configMap server logfile: {}.", getString(configMap, "logFile")));
        logger->setLogFile(getString(configMap, "logFile"));
        logger->trace(std::format("Setting new configMap server logLevel: {}.", getString(configMap, "logLevel")));
        logger->setSelectedLogSeverity(getString(configMap, "logLevel"));
        logger->trace(std::format("Setting new configMap server hostname: {}.", getString(configMap, "hostName")));
        datas->setServerHostName(getString(configMap, "hostName"));
        logger->trace(std::format("Setting new configMap server listen port: {}.", getInt(configMap, "port")));
        sock->setServerPort(getInt(configMap, "port"));
        logger->trace(std::format("Setting new configMap server sslEnabled: {}.", getBool(configMap, "sslEnabled")));
        this->setSslEnabled(getBool(configMap, "sslEnabled"));
        logger->trace(std::format("Setting new configMap server certificate file: {}.", getString(configMap, "certificateFile")));
        sslSock->setCertificateFile(getString(configMap, "certificateFile"));
        logger->trace(std::format("Setting new configMap server private key file: {}.", getString(configMap, "privateKeyFile")));
        sslSock->setPrivateKeyFile(getString(configMap, "privateKeyFile"));
    }catch(const std::exception &ex){
        std::string exMsg = std::format("httpdServer::getServerConfigParameters(): {}", ex.what());
        std::cerr << exMsg << std::endl;
        logger->error(exMsg);
    }
}


void httpdServer::processArguments(int argc, char **argv){
    try{
        logger->debug("Processing configuration arguments.");
        if(argc < 2){
            logger->trace("Arguments: No arguments given.");
            return;
        }
        std::vector<std::string> serverArguments;
        serverArguments.reserve(argc - 1);
        for(int i = 1; i < argc; i++){
            serverArguments.emplace_back(std::string(argv[i]));
        }
        static const std::vector<std::string> possibleArguments{"--config-file", "--help", "--log-file", "--usage", "--version"};
        for (size_t i = 0; i < serverArguments.size(); i++) {
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
                        serverRunningFlag = false;
                    }
                    if (arg == "--usage" || arg == "--help") {
                        logger->trace(std::format("Processing argument: {}", arg));
                        std::cout << getUsage() << std::endl;
                        serverRunningFlag = false;
                    }
                    if (arg.starts_with("--config-file")) {
                        std::string cliConfigFile = utils->split(arg, '=')[1];
                        logger->debug(std::format("Processing argument: {}", arg));
                        config->setConfigFile(cliConfigFile);
                        logger->debug(std::format("Setting config file: {}", cliConfigFile));
                    }
                    if (arg.starts_with("--log-file")) {
                        std::string cliLogFile = utils->split(arg, '=')[1];
                        logger->debug(std::format("Processing argument: {}", arg));
                        config->setLogFile(cliLogFile);
                        logger->debug(std::format("Setting log file: {}", cliLogFile));
                    }
                    break;
                }
            }
            if (!recognized) {
                logger->warning(std::format("Unknown argument given: {}", arg));
            }
        }
    }catch(std::runtime_error &ex){
        std::string exMsg = std::format("httpdServer::processArguments(): {}", ex.what());
        std::cerr << exMsg << std::endl;
        logger->error(exMsg);
    }
}


bool httpdServer::getServerRunningFlag() const{ return serverRunningFlag; }
void httpdServer::setSslEnabled(bool value){ sslEnabled = value; }

std::string httpdServer::getVersion() const {
    return "\n httpdServer (OS Windows)\n Version: 1.005\n Development: C++\n Author: Jan Novotny <bonoman@volny.cz> 2025\n";
}


std::string httpdServer::getUsage() const {
    return (
        getVersion() +
        "\nUsage: httpdServer [--command-switch[=value]]\n"
        "Available arguments:\n"
        " --help\n"
        " --usage\n"
        " --version\n"
        " --config-file=[\"Path to config file\"]\n"
        " --log-file=[\"Path to log file\"]"
        );
}
