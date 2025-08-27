#pragma once
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <mutex>

class Datas{
private:
	std::string url;
	std::string host;
	unsigned long long messageCounter;
	std::mutex messageCounterMutex;

public:
	Datas();
	void trim(std::string& message);
	std::string getSendMessage(std::string &clientId);
	std::string getServerResponse(std::string& message);
	void setRemoteHost(std::string& host);
	bool isInteger(const std::string& input);
	void incrementMessageCounter();
	unsigned long long getMessageCounter();
};
