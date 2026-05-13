#include <iostream>
#include <string>
#include <format>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <memory>
#include <filesystem>
#include <variant>


class Utils;
class FileManager;
class LogManager;


class ConfigManager {
private:
	std::string configFile = "./tariffServer.conf";
	std::shared_ptr<LogManager> logger;
	std::shared_ptr<FileManager> file;
	std::shared_ptr<Utils> utils;
	std::unordered_map<std::string, std::variant<std::string, int, bool, double>> configMap;
public:
	ConfigManager(std::shared_ptr<LogManager> logManager, std::shared_ptr<Utils> utilManager, std::shared_ptr<FileManager> fileManager);
	virtual ~ConfigManager() = default;
	void loadConfigValues();
	std::string getConfigFile() const;
	void setConfigFile(const std::string &configFile);
	void setLogFile(const std::string &logFile);
	void setHostName(const std::string &hostName);
	void setRemoteHost(const std::string &remoteHost);
	void setRemotePort(const int remotePort);
	void setRemoteUrl(const std::string &remoteUrl);
	void setLogLevel(const std::string &logLevel);
	void setThreadCount(const int threadCount);
	void setLaunchInterval(const int launchInterval);
	void parseConfigFile();
	const std::unordered_map<std::string, std::variant<std::string, int, bool, double>> &getConfigMap() const;
};