#include "data.h"

Dat::Dat() {
	{
		std::lock_guard<std::mutex> lockCounter(messageCounterMutex);
		messageCounter = 0;
	}
}

std::vector<uint8_t> Dat::getSendMessage() {
	std::vector<uint8_t> ntpRequestPacket(48, 0);
	ntpRequestPacket[0] = 0x23;
	return ntpRequestPacket;
}

void Dat::trim(std::string& message) {
	// Trim
	message.erase(0, message.find_first_not_of(" \t\n\r"));
	message.erase(message.find_last_not_of(" \t\n\r") + 1);
}

std::string Dat::getServerResponse(std::vector<uint8_t>& message) {
	std::string output;
	try {
		if (message.size() < 48) {
			throw std::runtime_error("getServerResponse(): Exception: Invalid NTP server response received. Too short.");
		}
		unsigned char firstByte = static_cast<unsigned char>(message[0]);
		if ((firstByte & 0x07) != 4) {
			throw std::runtime_error("getServerResponse(): Exception: Invalid NTP server response received. Unexpected mode.");
		}
		// convert NTP epoch (1900) to unix epoch (1970)
		uint32_t rcvdTimeStampSec = (message[40] << 24) | (message[41] << 16) | (message[42] << 8) | (message[43]);
		uint32_t rcvdSecFractional = (message[44] << 24) | (message[45] << 16) | (message[46] << 8) | (message[47]);
		const uint32_t NTP_TO_UNIX_EPOCH = 2208988800U;
		time_t unixTime = rcvdTimeStampSec - NTP_TO_UNIX_EPOCH;

		std::ostringstream oss;
		std::tm timeInfo;
		if (gmtime_s(&timeInfo, &unixTime) != 0) {
			throw std::runtime_error("getServerResponse() : Exception: Failed to reinterpret received time from server.");
		}
		double milliseconds = (rcvdSecFractional * 1000.0) / 4294967296.0;
		oss << std::put_time(&timeInfo, "%Y-%m-%d %H:%M:%S");
		oss << "." << std::setw(3) << std::setfill('0') << static_cast<int>(milliseconds);
		output = "NTP server time: " + oss.str() + " UTC";
	}
	catch (std::exception& ex) {
		throw std::runtime_error(ex.what());
	}
	return output;
}

void Dat::setRemoteHost(std::string& host) {
	this->host = host;
}

void Dat::incrementMessageCounter() {
	{
		std::lock_guard<std::mutex> lockCounter(messageCounterMutex);
		this->messageCounter++;
	}
}

unsigned long long Dat::getMessageCounter() {
	{
		std::lock_guard<std::mutex> lockCounter(messageCounterMutex);
		return this->messageCounter;
	}
}

bool Dat::isInteger(const std::string& input) {
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

std::string Dat::strip(const std::string& inputString, const std::string chars) {
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

std::vector<std::string> Dat::split(const std::string& inputString, char delimiter) {
	std::vector<std::string> output;
	std::stringstream ss(inputString);
	std::string token;
	while (getline(ss, token, delimiter)) {
		output.emplace_back(token);
	}
	return output;
}
