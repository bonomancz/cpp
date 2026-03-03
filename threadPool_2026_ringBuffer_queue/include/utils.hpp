#pragma once
#include <iostream>
#include <string>
#include <sstream>
#include <cctype>
#include <algorithm>
#include <vector>
#include <thread>
#include <random>
#include <variant>


class Utils{
public:
	static std::string strip(const std::string &inputString, const std::string &chars);
	static std::vector<std::string> split(const std::string &inputString, char delimiter);
	static bool isNumeric(const std::string& input);
	static std::string threadIdToString(std::thread::id threadId);
	bool isString(const std::string &s);
	bool isBoolean(const std::string &s);
	bool isFloat(const std::string &s);
	bool isInteger(const std::string &s);
	bool isPrime(const unsigned long long n);
	bool isEven(const unsigned long long n);
	bool isOdd(const unsigned long long n);
	int getRandomNumber(int from, int to);
};

