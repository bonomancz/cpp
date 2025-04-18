#pragma once
#include <iostream>
#include <string>
#include <mutex>
#include "file.h"

class Log {
private:
	File file;
	std::string logFile;
	int logLevel;
	mutable std::mutex logMutex;
public:
	Log(const std::string& logFile = "./logfile.log", int level = 7);
	void logger(const std::string& msecTime, int severity, const std::string& message);
	int getLoggingLevel() const;
	void setLoggingLevel(int logLevel);
	std::string getLoggingFile() const;
	void setLoggingFile(const std::string& logFile);
};

