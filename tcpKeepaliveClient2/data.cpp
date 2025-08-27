#include "data.h"

Datas::Datas() {
	{
		std::lock_guard<std::mutex> lockCounter(messageCounterMutex);
		messageCounter = 0;
	}
}

std::string Datas::getSendMessage(std::string &clientId) {
	{
		std::lock_guard<std::mutex> lockCounter(messageCounterMutex);
		this->messageCounter++;
		std::string sndMsg = "(ClientID: " + clientId + ") PING id:[" + std::to_string(this->messageCounter) + "]";
		return sndMsg;
	}
}

void Datas::trim(std::string& message) {
	// Trim
	message.erase(0, message.find_first_not_of(" \t\n\r"));
	message.erase(message.find_last_not_of(" \t\n\r") + 1);
}

std::string Datas::getServerResponse(std::string& message) {
	std::string line, output;
	bool responseSet = false;
	std::istringstream inStream(message);
	while (getline(inStream, line)) {
		this->trim(line);
		if (line.find("PONG") != std::string::npos && !responseSet) {
			output += "(Client side log) Server replied: " + message;
			responseSet = true;
			break;
		}
	}
	return output;
}

void Datas::setRemoteHost(std::string& host) {
	this->host = host;
}

void Datas::incrementMessageCounter() {
	{
		std::lock_guard<std::mutex> lockCounter(messageCounterMutex);
		this->messageCounter++;
	}
}

unsigned long long Datas::getMessageCounter() {
	{
		std::lock_guard<std::mutex> lockCounter(messageCounterMutex);
		return this->messageCounter;
	}
}

bool Datas::isInteger(const std::string& input) {
	try {
		size_t pos;
		int st = std::stoi(input, &pos);
		return pos == input.size();
	}
	catch (std::exception& e) {
		std::cerr << "Invalid argument: " << e.what() << std::endl;
		return false;
	}
}
