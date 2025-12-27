#include <iostream>
#include <string>
#include <list>
#include <regex>
#include <sstream>
#include <vector>

#pragma once

class DataManager{
private:
	std::string serverHostName{"DefaultBuildHostname"};
public:
	DataManager();
	~DataManager();
	std::string getHttpReponse(int httpResultCode);
	bool isHttpRequest(std::string &message);
	bool isExistingUrl(std::string &message);
	std::string getRequestUrl(std::string &message);
	std::string getClientRequest(std::string& message);
	std::string strip(const std::string& inputString, std::string chars);
	std::vector<std::string> split(const std::string& inputString, char delimiter);
	void setServerHostName(const std::string &hostName);
};

