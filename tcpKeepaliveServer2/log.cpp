#include "log.h"

Log::Log(const std::string& logFile, int level) : logFile(logFile), logLevel(level) {}

void Log::logger(const std::string& msecTime, int severity, const std::string& message) {
	if (severity > logLevel) return;
	std::string severityStr;
	switch (severity) {
		case 0:	severityStr = "EMERGENCY";
			break;
		case 1:	severityStr = "ALERT";
			break;
		case 2:	severityStr = "CRITICAL";
			break;
		case 3:	severityStr = "ERROR";
			break;
		case 4:	severityStr = "WARNING";
			break;
		case 5:	severityStr = "NOTICE";
			break;
		case 6:	severityStr = "INFO";
			break;
		case 7:	severityStr = "DEBUG";
			break;
		default:severityStr = "DEBUG";
			break;
	}
	std::string fileCopy;
	{
		std::lock_guard<std::mutex> lock(logMutex);
		fileCopy = logFile;
	}
	file.write(fileCopy, msecTime + " [" + severityStr + "]\t" + message, true);
}

int Log::getLoggingLevel() const { std::lock_guard<std::mutex> lock(logMutex); return logLevel; }
void Log::setLoggingLevel(int loggingLevel) { std::lock_guard<std::mutex> lock(logMutex); logLevel = loggingLevel; }

std::string Log::getLoggingFile() const { std::lock_guard<std::mutex> lock(logMutex); return logFile; }
void Log::setLoggingFile(const std::string& loggingFile) { std::lock_guard<std::mutex> lock(logMutex); logFile = loggingFile; }

