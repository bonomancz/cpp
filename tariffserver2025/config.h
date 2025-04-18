#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include "data.h"
#include "file.h"
#include "utils.h"

class Config {
private:
	File fl;
	std::unordered_map <std::string, std::string> defaultConfigMap;
	std::string app, ver, auth, year, email, defaultConfigFile;
	int listenPort = 9002;
	int mysqlPort = 3306;
	std::string mysqlTable = "mysql_table";
	std::string mysqlDatabase = "mysql_database";
	std::string mysqlUser = "user";
	std::string mysqlPassword = "password";
	std::string mysqlServer = "localhost";
public:
	Config();
	std::string getVersion();
	std::string getHelp();
	void getUsage(const std::string &errorMsg);
	std::unordered_map <std::string, std::string> &buildServiceConfigMap();
	void processExecParams(int argc, char* argv[]);
	void parseServiceConfigMap(std::unordered_map<std::string, std::string>& serviceConfigMap);
	int get_listenPort();
	int get_mysqlPort();
	std::string get_mysqlTable();
	std::string get_mysqlDatabase();
	std::string get_mysqlUser();
	std::string get_mysqlPassword();
	std::string get_mysqlServer();
};
