#include "../include/configManager.hpp"
#include "../include/logManager.hpp"


ConfigManager::ConfigManager(std::shared_ptr<LogManager> logMgr) : logger(logMgr) {
    logger->debug("ConfigManager initialized.");
}


ConfigManager::~ConfigManager() {}


void ConfigManager::loadConfigValues() {
    try{
        logger->debug("Loading configuration parameters from config file.");
        if(configFile.empty()){
            std::string msg = "No config file specified. Using default values."; 
            logger->warning(msg);
            throw std::runtime_error(msg);
        }
        if(!std::filesystem::exists(configFile)){
            std::string msg = std::format("Config file {} not available. Using default values.", configFile); 
            logger->error(msg);
            throw std::runtime_error(msg);
        }
        std::string msg = std::format("Config file {} found. Reading new config values.", configFile);
        logger->debug(msg);
        parseConfigFile();
    }catch(const std::exception &ex){
        std::string exMsg = std::format("ConfigManager::loadConfigValues(): {}", ex.what());
        std::cerr << exMsg << std::endl;
        logger->error(exMsg);
        // throw;
    }
}


void ConfigManager::parseConfigFile(){
    try{
        if(!std::filesystem::exists(configFile)){
            std::string msg = std::format("Config file {} not available. Using default values.", configFile); 
            logger->error(msg);
        }
        std::ifstream confFile(configFile);
        nlohmann::json cfg;
        confFile >> cfg;
        configMap.clear();
        auto parseSection = [&](const std::string& sectionName) {
            if (!cfg.contains(sectionName)) {
                logger->error(std::format("Missing \"{}\" section in config file.", sectionName));
                return;
            }
            for (auto& [key, value] : cfg[sectionName].items()) {
                logger->debug(std::format("Config value {}: {}: {}", sectionName, key, value.dump()));                
                if (value.is_string()) {
                    configMap[key] = value.get<std::string>();
                } else if (value.is_number_integer()) {
                    configMap[key] = value.get<int>();
                } else if (value.is_boolean()) {
                    configMap[key] = value.get<bool>();
                } else if (value.is_number_float()) {
                    configMap[key] = value.get<double>();
                } else {
                    logger->warning(std::format("Unsupported JSON type for key: {} — storing as string.", key));
                    configMap[key] = value.dump();
                }
            }
        };
        parseSection("server");
        parseSection("ssl");
        parseSection("logging");
        logger->debug("Parsing of config file finished.");
    }catch(std::exception &ex){
        std::string exMsg = std::format("ConfigManager::parseConfigFile(): {}", ex.what());
        std::cerr << exMsg << std::endl;
        logger->error(exMsg);
    }
}

void ConfigManager::setLogFile(const std::string &logFl){ configMap["logfile"] = logFl; }
void ConfigManager::setConfigFile(const std::string &confFl){ configFile = confFl; }
void ConfigManager::setHostName(const std::string &hostName){ configMap["hostName"] = hostName; }
std::string ConfigManager::getConfigFile() const { return configFile; }
const std::unordered_map<std::string, std::variant<std::string, int, bool, double>> &ConfigManager::getConfigMap() const{ return configMap; }
