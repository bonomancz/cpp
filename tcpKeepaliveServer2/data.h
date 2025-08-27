#pragma once
#include <iostream>
#include <string>
#include <list>
#include <regex>
#include <sstream>
#include <vector>

class Data{
private:
	std::string host;
public:
	std::string getSendMessage(const std::string& message);
	bool isPingRequest(const std::string &message);
	std::string getClientRequest(const std::string& message);
	std::string getClientId(const std::string& message);
	std::string strip(const std::string& inputString, const std::string chars);
	std::vector<std::string> split(const std::string& inputString, char delimiter);
	bool isInteger(const std::string& input);
	void setRemoteHost(std::string& host);
};

