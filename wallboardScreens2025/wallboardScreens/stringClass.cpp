#include "stringClass.h"

std::string StrManip::charToStr(char* chr) {
	std::stringstream ss;
	std::string str;
	ss << chr;
	ss >> str;
	return str;
}

int StrManip::getLen(std::string str) {
	int len = static_cast<int>(str.length());
	return len;
}

std::vector<std::string> StrManip::getToks(std::string str) {
	std::vector<std::string> arr;
	std::istringstream iss(str);
	std::string temp;
	while (iss >> temp) {
		arr.push_back(temp);
	}
	return arr;
}

void StrManip::printToks(std::vector<std::string> strArr){
	int x = static_cast<int>(strArr.size());
	for (int i = 0; i < x; i++) {
		std::cout << strArr[i] << " ";
	}
	std::cout << std::endl;
}

std::string StrManip::removeWhitespace(std::string st) {
	std::string retVal, temp;
	std::istringstream iss(st);
	if (iss >> temp) {
		retVal += temp;
	}
	return retVal;
}

std::string StrManip::strip(const std::string& inputString, std::string chars) {
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

std::vector<std::string> StrManip::split(const std::string& inputString, char delimiter) {
	std::vector<std::string> output;
	std::stringstream ss(inputString);
	std::string token;
	while (getline(ss, token, delimiter)) {
		output.emplace_back(token);
	}
	return output;
}
