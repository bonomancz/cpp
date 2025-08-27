#pragma once
#include <iostream>
#include <string>
#include <mutex>
#include "file.h"

class Log{
private:
	File file;
	int logLevel;
	std::string logFile;
	mutable std::mutex logMutex;
public: 
	Log(const std::string& logFile = ".\\tcpKeepaliveServer2.log", int level = 7);
	void logger(const std::string& msecTime, int severity, const std::string& message);
	int getLoggingLevel() const;
	void setLoggingLevel(int logLevel);
	std::string getLoggingFile() const;
	void setLoggingFile(const std::string& logFile);
};

