#include <iostream>
#include <string>
#include <format>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <memory>
#include <filesystem>
#include <variant>
#include "nlohmann/json.hpp"

class LogManager;

class ConfigManager {
private:
	std::string configFile = "./defaultBuildServerConfig.json";
	std::shared_ptr<LogManager> logger;
	std::unordered_map<std::string, std::variant<std::string, int, bool, double>> configMap;
public:
	ConfigManager(std::shared_ptr<LogManager> logManager);
	~ConfigManager();
	void loadConfigValues();
	std::string getConfigFile() const;
	void setConfigFile(const std::string &configFile);
	void setLogFile(const std::string &logFile);
	void setHostName(const std::string &hostName);
	void parseConfigFile();
	const std::unordered_map<std::string, std::variant<std::string, int, bool, double>> &getConfigMap() const;
};