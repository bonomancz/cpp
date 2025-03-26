#include "config.h"

Config::Config() {
	app = "Wallboard screens threads";
	ver = "v1.0.6";
	auth = "Jan Novotny";
	year = "2016-2025(20250324)";
	email = "jan.novotny.cz@gmail.com";
	configFile = "c:\\wallboardsTool\\wallboardScreens.conf";
	service_loop_timeout = 8000;
	service_thread_timeout = 6000;
	service_browser_path = "C:\\Program Files (x86)\\Microsoft\\Edge\\Application\\msedge.exe";
}

const std::vector<std::string> Config::getConfiguredWallboards(){
	return this->configuredWallboards;
}

std::string Config::getVersion() {
	std::string retVal = app + " " + ver + ", " + auth + " " + year + " (" + email + ")";
	return retVal;
}

std::string Config::getHelp() {
	std::string helpString = getVersion();
	helpString += "\nUsage: wallboardscreens [--parameter] (see parameter list below)\n";
	helpString += "\nBy default application reads config file wallboardScreens.conf in the same directory";
	helpString += "\n--config\t\tuse given configuration file [--config=/path/to/config/wallboardScreens.conf]";
	helpString += "\n--version\t\tshow application version and exit";
	helpString += "\n--help\t\t\tshow this help and exit";
	helpString += "\n";
	return helpString;
}

void Config::getUsage(std::string errorMsg) {
	std::cout << getHelp();
	throw std::runtime_error(errorMsg);
}

std::unordered_map<std::string, std::string> Config::getWallboardConfig() {
	std::unordered_map<std::string, std::string> configValues;
	try {
		bool wallboardsFound = false;
		bool serviceFound = false;
		std::vector<std::string>redConfigLine;
		for(auto &wbConfigLine : fl.read(configFile)){
			std::string stripped = str.strip(wbConfigLine, " ");
			if (wbConfigLine.empty()) { continue; }
			if (stripped[0] == '#' || stripped[0] == ';') { continue; }
			if (wbConfigLine.find("[service]") != std::string::npos) {
				serviceFound = true;
				continue;
			}
			if (wbConfigLine.find("[wallboards]") != std::string::npos) {
				wallboardsFound = true;
				continue;
			}	
			if (serviceFound) {
				if (!wbConfigLine.empty() && wbConfigLine.find("=") != std::string::npos && wbConfigLine.find("service") != std::string::npos) {
					redConfigLine = str.split(wbConfigLine, '=');
					if (redConfigLine.size() >= 2) {
						configValues.insert({ str.strip(redConfigLine[0], " "), str.strip(redConfigLine[1], " ") });
					}
				}
			}
			if (wallboardsFound) {
				if (!wbConfigLine.empty() && wbConfigLine.find("|") != std::string::npos && wbConfigLine.find("wallboard") != std::string::npos) {
					redConfigLine = str.split(wbConfigLine, '|');
					if (redConfigLine.size() >= 2) {
						configValues.insert({ str.strip(redConfigLine[0], " "), str.strip(redConfigLine[1], " ") });
					}
				}
			}
		}
		if (!serviceFound) {
			throw std::runtime_error("wallboardScreens: getConfig(): Exception in config file. Need config section [service] in config file " + configFile);
		}
		if (!wallboardsFound) {
			throw std::runtime_error("wallboardScreens: getConfig(): Exception in config file. Need config section [wallboards] in config file " + configFile);
		}
		return configValues;
	}
	catch (std::ifstream::failure& e) {
		std::cout << "wallboardScreens: getConfig(): Exception in reading config file " + configFile << e.what() << std::endl;
		std::cout << "wallboardScreens: getConfig(): Closing config file." << std::endl;
		std::exit(1);
	}
	catch (std::exception& e) {
		std::cout << "wallboardScreens: getConfig(): Exception in config file " + configFile << std::endl << e.what() << std::endl;
		std::cout << "wallboardScreens: getConfig(): Closing config file." << std::endl;
		std::exit(1);
	}
	return {};
}

void Config::getWallboardParams(int argc, char* argv[]) {
	if (argc > 1) {
		for (int i = 1; i < argc; i++) {
			std::string arg = str.charToStr(argv[i]);
			if (arg.find("config=") != std::string::npos) {
				auto parts = str.split((std::string)argv[i], '=');
				if (parts.size() > 1) {
					if (parts[1].empty()) {
						throw std::runtime_error("Exception: Filename in given argument is empty.");
					}
					configFile = str.strip(parts[1], " ");
				}
			}
			else if (arg.find("version") != std::string::npos) {
				std::cout << getVersion() << std::endl;
				exit(0);
			}
			else if (arg.find("help") != std::string::npos) {
				std::cout << getHelp();
				exit(0);
			}
			else {
				getUsage("Exception: Bad application parameter given.");
			}
		}
	}
	if (!fl.fileExists(configFile)) {
		throw std::runtime_error("Exception: Config file doesn't exist. " + configFile);
	}
	std::unordered_map<std::string, std::string> wbConfigMap = getWallboardConfig();
	for(auto& configuredWb : wbConfigMap){
		if (configuredWb.first.find("wallboard") != std::string::npos) {
			configuredWallboards.push_back(configuredWb.second);
		}
	}
	this->parseWallboardConfigMap(wbConfigMap);
}

void Config::parseWallboardConfigMap(std::unordered_map<std::string, std::string>& wbdConfMap) {
	for (auto& item : wbdConfMap) {
		if (item.first.find("service_browser_path") != std::string::npos && !item.second.empty()) {
			this->service_browser_path = item.second;
		}
		if (item.first.find("service_loop_timeout") != std::string::npos && !item.second.empty()) {
			int loopTimeout = stoi(item.second);
			if (loopTimeout > 0) {
				this->service_loop_timeout = loopTimeout;
			}
		}
		if (item.first.find("service_thread_timeout") != std::string::npos && !item.second.empty()) {
			int threadTimeout = stoi(item.second);
			if (threadTimeout > 0) {
				this->service_thread_timeout = threadTimeout;
			}
		}
	}
}

void Config::set_Service_loop_timeout(int timeout) { this->service_loop_timeout = timeout; }
void Config::set_Service_thread_timeout(int timeout) { this->service_thread_timeout = timeout; }
void Config::set_Service_browser_path(const std::string& browserPath) { this->service_browser_path = browserPath; }
int Config::get_Service_loop_timeout() { return this->service_loop_timeout; }
int  Config::get_Service_thread_timeout() { return this->service_thread_timeout; }
const std::string Config::get_Service_browser_path() { return this->service_browser_path; }
