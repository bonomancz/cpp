#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include "stringClass.h"
#include "fileClass.h"

class Config{
private:
	int service_loop_timeout, service_thread_timeout;
	std::string app, ver, auth, year, email, configFile, service_browser_path;
	std::vector<std::string> configuredWallboards;
	StrManip str;
	FileManip fl;

public:
	Config();
	const std::vector<std::string> getConfiguredWallboards();
	std::string getVersion();
	std::string getHelp();
	void getUsage(std::string errorMsg);
	std::unordered_map <std::string, std::string> getWallboardConfig();
	void getWallboardParams(int argc, char* argv[]);
	void parseWallboardConfigMap(std::unordered_map<std::string, std::string>& wbdConfMap);
	void set_Service_thread_timeout(int timeout);
	void set_Service_loop_timeout(int timeout);
	void set_Service_browser_path(const std::string& browserPath);
	int get_Service_thread_timeout();
	int get_Service_loop_timeout();
	const std::string get_Service_browser_path();
};
