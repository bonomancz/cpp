#pragma once
#include <iostream>
#include <string>
#include <list>
#include <regex>
#include <sstream>
#include <vector>
#include <deque>
#include <optional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "time.h"
#include "mysqldb.h"

struct TariffSentence {
	int type;
	std::string date;
	std::string dbDateTime;
	std::string time;
	std::string dnis;
	std::string ani;
	std::string trunk;
	int duration;
};

class Config;

class Data {
private:
	Time tm;
	MysqlDb mysqldb;
	std::deque<TariffSentence> queueBuffer;
	std::mutex queueBufferMutex;
	std::condition_variable queueCv;
	static const std::regex tariffSentencePattern;
public:
	void processRcvdMessage(const std::string message);
	void insertDataToBuffer(TariffSentence tariffData);
	void processBufferToStorage();
	bool isValidTariffSentence(const std::string& message);
	std::optional<TariffSentence> parseTariffSentence(const std::string& sentence);
	bool isDataStorageInitialized(Config &conf);
	bool pushToStorage(TariffSentence& sentence);
};
