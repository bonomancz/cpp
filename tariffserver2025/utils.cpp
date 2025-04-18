#include "utils.h"


std::string Utils::strip(const std::string& inputString, const std::string chars) {
	try {
		size_t begin = inputString.find_first_not_of(chars);
		if (begin == std::string::npos) {
			return "";
		}
		size_t end = inputString.find_last_not_of(chars);
		return inputString.substr(begin, end - begin + 1);
	}
	catch (std::runtime_error& rex) {
		std::cerr << rex.what() << std::endl;
		return {};
	}
}


std::vector<std::string> Utils::split(const std::string& inputString, char delimiter) {
	std::vector<std::string> output;
	std::stringstream ss(inputString);
	std::string token;
	while (getline(ss, token, delimiter)) {
		output.emplace_back(token);
	}
	return output;
}


bool Utils::isInteger(const std::string& input) {
	try {
		size_t pos;
		std::stoi(input, &pos);
		return pos == input.size();
	}
	catch (std::exception& e) {
		std::cerr << "Invalid argument: " << e.what() << std::endl;
		return false;
	}
}


std::string Utils::threadIdToString(std::thread::id threadId) {
	std::ostringstream oss;
	oss << threadId;
	return oss.str();
}