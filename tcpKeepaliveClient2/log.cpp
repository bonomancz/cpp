#include "log.h"

Log::Log(const std::string& logFile, int level) : logFile(logFile), logLevel(level) {}

void Log::logger(const std::string& msecTime, int severity, const std::string& message) {
	if (severity > logLevel) return;
	std::lock_guard<std::mutex> lock(logMutex);
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
	file.write(logFile, msecTime + " [" + severityStr + "]\t" + message, true);
}

int Log::getLoggingLevel() const { return logLevel; }
void Log::setLoggingLevel(int loggingLevel) { logLevel = loggingLevel; }

std::string Log::getLoggingFile() const { return logFile; }
void Log::setLoggingFile(const std::string& loggingFile) { logFile = loggingFile; }

