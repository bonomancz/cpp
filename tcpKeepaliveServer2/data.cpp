#include "data.h"

std::string Data::getSendMessage(const std::string &message) {
	std::string sendMessage = "PONG id:[" + this->getClientRequest(message) + "]";
	return sendMessage;
}

bool Data::isPingRequest(const std::string &message) {
	bool output = false;
	std::regex pingRequestPattern(R"(PING id:\[\d+\])");
	if (regex_search(message, pingRequestPattern)) {
		output = true;
	}
	return output;
}

std::string Data::getClientRequest(const std::string& message) {
	std::string output;
	std::regex pingRequestPattern(R"(PING id:\[(\d+)\])");
	std::smatch numberMatch;
	if (regex_search(message, numberMatch, pingRequestPattern)) {
		output = numberMatch[1];
	}
	return output;
}

std::string Data::strip(const std::string& inputString, const std::string chars) {
	try {
		size_t begin = inputString.find_first_not_of(chars);
		if (begin == std::string::npos) {
			return "";
		}
		size_t end = inputString.find_last_not_of(chars);
		return inputString.substr(begin, end - begin + 1);
	}
	catch (std::runtime_error rex) {
		std::cerr << rex.what() << std::endl;
		return "";
	}
}

std::vector<std::string> Data::split(const std::string& inputString, char delimiter) {
	std::vector<std::string> output;
	std::stringstream ss(inputString);
	std::string token;
	while (getline(ss, token, delimiter)) {
		output.emplace_back(token);
	}
	return output;
}

void Data::setRemoteHost(std::string& host) {
	this->host = host;
}

bool Data::isInteger(const std::string& input) {
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