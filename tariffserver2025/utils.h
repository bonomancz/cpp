#pragma once
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <thread>


class Utils{
public:
	static std::string strip(const std::string& inputString, const std::string chars);
	static std::vector<std::string> split(const std::string& inputString, char delimiter);
	static bool isInteger(const std::string& input);
	static std::string threadIdToString(std::thread::id threadId);
};

