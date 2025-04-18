#include "config.h"
#include "utils.h"


Config::Config() {
	app = "Tariffserver 2025";
	ver = "v2.0.1";
	auth = "Jan Novotny";
	year = "2010-2025(20250409)";
	email = "jan.novotny.cz@gmail.com";
	defaultConfigFile = "./tariffserver.conf";
}

std::string Config::getVersion() {
	std::string retVal = app + " " + ver + ", " + auth + " " + year + " <" + email + ">";
	return retVal;
}

std::string Config::getHelp() {
	std::string helpString = getVersion();
	helpString += "\nUsage: tariffserver [--parameter] (see parameters list below)\n";
	helpString += "\nBy default service reads config file mysql.conf in the same directory";
	helpString += "\n--mysql-config\t\tuse given configuration file [--mysql-config=/path/to/config/mysql.conf]";
	helpString += "\n--version\t\tshow application version and exit";
	helpString += "\n--help\t\t\tshow this help and exit";
	return helpString;
}

void Config::getUsage(const std::string &errorMsg) {
	throw std::runtime_error(errorMsg + "\n" + getHelp());
}

std::unordered_map<std::string, std::string>& Config::buildServiceConfigMap() {
	try {
		this->defaultConfigMap.clear();
		bool mysqlFound = false;
		bool netFound = false;
		std::string currentSection;
		std::vector<std::string>redConfigLine;
		for (auto& srvConfigLine : fl.read(this->defaultConfigFile)) {
			std::string stripped = Utils::strip(srvConfigLine, " ");
			if (srvConfigLine.empty()) { continue; }
			if (stripped[0] == '#' || stripped[0] == ';') { continue; }
			if (srvConfigLine.find("[mysql]") != std::string::npos) {
				currentSection = "mysql";
				mysqlFound = true;
				continue;
			}
			if (srvConfigLine.find("[net]") != std::string::npos) {
				currentSection = "net";
				netFound = true;
				continue;
			}
			if (currentSection == "mysql" || currentSection == "net") {
				if (!srvConfigLine.empty() && srvConfigLine.find("=") != std::string::npos) {
					redConfigLine = Utils::split(srvConfigLine, '=');
					if (redConfigLine.size() >= 2) {
						this->defaultConfigMap.insert({ Utils::strip(redConfigLine[0], " "), Utils::strip(redConfigLine[1], " ") });
					}
				}
			}
		}
		if (!mysqlFound) {
			throw std::runtime_error("Config::getServiceConfigMap(): Exception in config file. Need config section [mysql] in config file " + this->defaultConfigFile);
		}
		if (!netFound) {
			throw std::runtime_error("Config::getServiceConfigMap(): Exception in config file. Need config section [net] in config file " + this->defaultConfigFile);
		}
	}
	catch (std::ifstream::failure& e) {
		std::cout << "Config::getServiceConfigMap(): Exception in reading config file " + this->defaultConfigFile << e.what() << std::endl;
		std::cout << "Config::getServiceConfigMap(): Closing config file." << std::endl;
		throw;
	}
	catch (std::exception& e) {
		std::cout << "Config::getServiceConfigMap(): Exception in config file " + this->defaultConfigFile << std::endl << e.what() << std::endl;
		std::cout << "Config::getServiceConfigMap(): Closing config file." << std::endl;
		throw;
	}
	return this->defaultConfigMap;
}

void Config::processExecParams(int argc, char* argv[]) {
	try {
		if (argc > 1) {
			for (int i = 1; i < argc; i++) {
				std::string arg = std::string(argv[i]);
				if (arg.find("--config=") != std::string::npos) {
					auto parts = Utils::split((std::string)argv[i], '=');
					if (parts.size() > 1) {
						if (parts[1].empty()) {
							throw std::runtime_error("Exception: Filename in given argument is empty.");
						}
						this->defaultConfigFile = Utils::strip(parts[1], " ");
					}
				}
				else if (arg.find("version") != std::string::npos) {
					throw std::runtime_error(getVersion());
				}
				else if (arg.find("help") != std::string::npos) {
					throw std::runtime_error(getHelp());
				}
				else {
					getUsage("Error: Bad parameter given.");
				}
			}
		}
		if (!fl.fileExists(this->defaultConfigFile)) {
			throw std::runtime_error("Exception: Config file doesn't exist. " + this->defaultConfigFile);
		}
		this->parseServiceConfigMap(buildServiceConfigMap());
	}
	catch (std::runtime_error &ex) {
		throw;
	}
}

void Config::parseServiceConfigMap(std::unordered_map<std::string, std::string>& serviceConfigMap) {
	for (auto& item : serviceConfigMap) {
		if (item.first.find("net_listen_port") != std::string::npos && !item.second.empty()) {
			this->listenPort = std::stoi(item.second);
		}
		if (item.first.find("mysql_port") != std::string::npos && !item.second.empty()) {
			this->mysqlPort = stoi(item.second);
		}
		if (item.first.find("mysql_table") != std::string::npos && !item.second.empty()) {
			this->mysqlTable = item.second;
		}
		if (item.first.find("mysql_database") != std::string::npos && !item.second.empty()) {
			this->mysqlDatabase = item.second;
		}
		if (item.first.find("mysql_password") != std::string::npos && !item.second.empty()) {
			this->mysqlPassword = item.second;
		}
		if (item.first.find("mysql_user") != std::string::npos && !item.second.empty()) {
			this->mysqlUser = item.second;
		}
		if (item.first.find("mysql_server") != std::string::npos && !item.second.empty()) {
			this->mysqlServer = item.second;
		}
	}
}

int Config::get_listenPort() { return this->listenPort; }
int Config::get_mysqlPort() { return this->mysqlPort; }
std::string Config::get_mysqlTable() { return this->mysqlTable; }
std::string Config::get_mysqlDatabase() { return this->mysqlDatabase; }
std::string Config::get_mysqlUser() { return this->mysqlUser; }
std::string Config::get_mysqlPassword() { return this->mysqlPassword; }
std::string Config::get_mysqlServer() { return this->mysqlServer; }
