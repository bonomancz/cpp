#pragma once
#include <iostream>
#include <string>
#include <string_view>
#include <sstream>
#include <cctype>
#include <algorithm>
#include <vector>
#include <thread>
#include <random>
#include <variant>
#include <memory>
#include <chrono>
#include <format>
#include <regex>


class ChronoTime;
struct tariffSentenceData;


class Utils{
private:
	std::shared_ptr<ChronoTime> timer;
	std::shared_ptr<tariffSentenceData> tData;
	static inline const std::regex tariffSentencePattern{R"(^\s*(\d)\s+(\d{6})\s+(\d{4})\s+([0-9#]+)\s+([0-9#]+)\s+(?:(\d{4})\s+)?(\d{5})\s*$)"};
public:
	Utils(std::shared_ptr<ChronoTime> tmr, std::shared_ptr<tariffSentenceData> tdata);
	static std::string_view strip(std::string_view inputString, std::string_view chars);
	static std::vector<std::string_view> split(std::string_view input, char delimiter);
	static bool isNumeric(const std::string& input);
	static std::string threadIdToString(std::thread::id threadId);
	tariffSentenceData parseTariffSentence(std::string_view inputString);
	int stringViewToInt(std::string_view input);

	bool isValidTariffSentence(const std::string_view sentence);
	bool isString(const std::string &s);
	bool isBoolean(const std::string &s);
	bool isFloat(const std::string &s);
	bool isInteger(const std::string &s);
	bool isPrime(const unsigned long long n);
	bool isEven(const unsigned long long n);
	bool isOdd(const unsigned long long n);
	int getRandomNumber(int from, int to);
};

