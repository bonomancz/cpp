#include "../include/utils.hpp"
#include "../include/fileManager.hpp"
#include "../include/logManager.hpp"
#include "../include/configManager.hpp"


ConfigManager::ConfigManager(std::shared_ptr<LogManager> logMgr, std::shared_ptr<Utils> utilMgr, std::shared_ptr<FileManager> flMgr) : logger(logMgr), utils(utilMgr), file(flMgr) {
    logger->debug("ConfigManager initialized.");
}


void ConfigManager::loadConfigValues() {
    try{
        logger->debug("Loading configuration parameters from config file.");
        if(configFile.empty()){
            std::string msg = "No config file specified. Using default values."; 
            logger->debug(msg);
            throw std::runtime_error(msg);
        }
        if(!std::filesystem::exists(configFile)){
            std::string msg = std::format("Config file {} not available. Using default values.", configFile); 
            throw std::runtime_error(msg);
        }
        std::string msg = std::format("Config file {} found. Reading new config values.", configFile);
        logger->debug(msg);
        parseConfigFile();
    }catch(const std::exception &ex){
        std::string exMsg = std::format("ConfigManager: {}", ex.what());
        logger->debug(exMsg);
    }
}


void ConfigManager::parseConfigFile(){
    try{
        if(!file->fileExists(configFile)){
            std::string msg = std::format("Config file {} not available. Using default values.", configFile); 
            logger->error(msg);
        }
        auto parseSection = [&](const std::string& sectionName) {
            bool haveValidSection = false;
            std::string sectionToFind = std::format("[{}]", sectionName);
            std::vector<std::string> configData = file->read(configFile);
            logger->debug(std::format("ConfigManager::parseConfigFile() Searching for section: {}", sectionToFind));            

            for(const auto &configLine : configData){
                if(configLine.starts_with(";") || configLine.starts_with("#") || configLine.empty()){ continue; }
                if(configLine.find(sectionToFind) != std::string::npos) {
                    haveValidSection = true;
                    logger->trace(std::format("ConfigManager::parseConfigFile() Found valid section: {}", sectionToFind));
                    continue;
                }
                if(configLine.find("[") != std::string::npos && haveValidSection){ break; }
                if(haveValidSection){
                    bool willInsertToConfigMap = false;
                    logger->debug(configLine);
                    std::vector<std::string_view> redConfigLine;
                    if(sectionName == "general"){
				        if (configLine.find("=") != std::string::npos) {
					        if((redConfigLine = utils->split(configLine, '=')).size() > 1){ willInsertToConfigMap = true; }
				        }
                    }else if(sectionName == "mysql"){
				        if (configLine.find("=") != std::string::npos) {
					        if((redConfigLine = utils->split(configLine, '=')).size() > 1){ willInsertToConfigMap = true; }
				        }
                    }else if(sectionName == "net"){
				        if (configLine.find("=") != std::string::npos) {
					        if((redConfigLine = utils->split(configLine, '=')).size() > 1){ willInsertToConfigMap = true; }
				        }
                    }else if(sectionName == "log"){
				        if (configLine.find("=") != std::string::npos) {
					        if((redConfigLine = utils->split(configLine, '=')).size() > 1){ willInsertToConfigMap = true; }
				        }
                    }

                    if (willInsertToConfigMap) {
                        std::string_view rawKey = redConfigLine[0];
                        std::string_view rawValue = redConfigLine[1];
                        std::string_view key = utils->strip(rawKey, " ");
                        std::string_view value = utils->strip(rawValue, " ");
                        if(utils->isString(std::string(value))) {
                            logger->trace(std::format("ConfigMap inserting string key: {}: {}", key, value));
                            configMap.insert({ std::string(key), std::string(value) });
                        }else if(utils->isInteger(std::string(value))) {
                            logger->trace(std::format("ConfigMap inserting integer key: {}: {}", key, value));
                            configMap.insert({ std::string(key), std::stoi(std::string(value)) });
                        }else if(utils->isBoolean(std::string(value))) {
                            logger->trace(std::format("ConfigMap inserting bool key: {}: {}", key, value));
                            bool b = (value == "true" || value == "1");
                            configMap.insert({ std::string(key), b });
                        }else if(utils->isFloat(std::string(value))) {
                            logger->trace(std::format("ConfigMap inserting float key: {}: {}", key, value));
                            configMap.insert({ std::string(key), std::stod(std::string(value)) });
                        }else{
                            logger->trace(std::format("Unsupported type key: {}: {}", key, value));
                            configMap.insert({ std::string(key), std::string(value) });
                        }
                        logger->trace(std::format("ConfigManager::parseConfigFile() Config map insert parsed parameter: {}: {}", key, value));
                    }
                }
            }
        };
        parseSection("general");
        parseSection("mysql");
        parseSection("net");
        parseSection("log");
        logger->debug("Parsing config file sections finished.");
    }catch(std::exception &ex){
        std::string exMsg = std::format("ConfigManager::parseConfigFile(): {}", ex.what());
        std::cerr << exMsg << std::endl;
        logger->error(exMsg);
    }
}

void ConfigManager::setLogFile(const std::string &logFl){ configMap["logfile"] = logFl; }
void ConfigManager::setConfigFile(const std::string &confFl){ configFile = confFl; }
void ConfigManager::setHostName(const std::string &hostName){ configMap["hostName"] = hostName; }
void ConfigManager::setRemoteHost(const std::string &remoteHost){ configMap["remoteHost"] = remoteHost; }
void ConfigManager::setRemotePort(const int remotePort){ configMap["remotePort"] = remotePort; }
void ConfigManager::setRemoteUrl(const std::string &remoteUrl){ configMap["remoteUrl"] = remoteUrl; }
void ConfigManager::setLogLevel(const std::string &logLevel){ configMap["logLevel"] = logLevel; }
void ConfigManager::setThreadCount(const int threadCount){ configMap["threadCount"] = threadCount; }
void ConfigManager::setLaunchInterval(const int launchInterval){ configMap["launchInterval"] = launchInterval; }
std::string ConfigManager::getConfigFile() const { return configFile; }
const std::unordered_map<std::string, std::variant<std::string, int, bool, double>> &ConfigManager::getConfigMap() const{ return configMap; }
