#pragma once
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <mutex>
#include <iomanip>
#include <ctime>

class Dat{
private:
	std::string url;
	std::string host;
	unsigned long long messageCounter;
	std::mutex messageCounterMutex;

public:
	Dat();
	void trim(std::string& message);
	std::vector<uint8_t> getSendMessage();
	std::string getServerResponse(std::vector<uint8_t>& message);
	void setRemoteHost(std::string& host);
	bool isInteger(const std::string& input);
	void incrementMessageCounter();
	unsigned long long getMessageCounter();
	std::string strip(const std::string& inputString, const std::string chars);
	std::vector<std::string> split(const std::string& inputString, char delimiter);
};
