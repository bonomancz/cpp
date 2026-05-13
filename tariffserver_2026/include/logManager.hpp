#pragma once
#include <iostream>
#include <string>
#include <format>
#include <memory>
#include <unordered_map>


class FileManager;
class ChronoTime;
class threadPool;

enum class LoggingDestination { Off, Stdout, File, Syslog, Database, Gui };

enum class LoggingSeverity { Trace, Debug, Info, Notice, Warning, Error, Critical, Alert, Emergency };

class LogManager{
private:
    LoggingDestination selectedLogType = LoggingDestination::Stdout;
    LoggingSeverity selectedLogSeverity = LoggingSeverity::Error;
    std::shared_ptr<FileManager> file;
    std::shared_ptr<ChronoTime> timer;
    std::shared_ptr<threadPool> pool;
    std::string logFile = "./tariffServer.log";
    inline static const std::unordered_map<std::string, LoggingSeverity> severityMap{
        {"trace", LoggingSeverity::Trace},
        {"debug", LoggingSeverity::Debug},
        {"info", LoggingSeverity::Info},
        {"notice", LoggingSeverity::Notice},
        {"warning", LoggingSeverity::Warning},
        {"error", LoggingSeverity::Error},
        {"critical", LoggingSeverity::Critical},
        {"alert", LoggingSeverity::Alert},
        {"emergency", LoggingSeverity::Emergency}
    };
    inline static const std::unordered_map<std::string, LoggingDestination> logTypeMap{
        {"off", LoggingDestination::Off},
        {"stdout", LoggingDestination::Stdout},
        {"file", LoggingDestination::File},
        {"syslog", LoggingDestination::Syslog},
        {"database", LoggingDestination::Database},
        {"gui", LoggingDestination::Gui}
    };
public:
    LogManager(std::shared_ptr<ChronoTime> tmr, std::shared_ptr<FileManager> fl);
    ~LogManager();
    void trace(const std::string_view logMessage);
    void debug(const std::string_view logMessage);
    void info(const std::string_view logMessage);
    void notice(const std::string_view logMessage);
    void warning(const std::string_view logMessage);
    void error(const std::string_view logMessage);
    void critical(const std::string_view logMessage);
    void alert(const std::string_view logMessage);
    void emergency(const std::string_view logMessage);
    void pushLogQueue(const LoggingSeverity logSeverity, const std::string_view logMessage);
    void processLogQueue(const std::string &logMessage);
    LoggingSeverity stringToSeverity(const std::string &configSeverity);
    LoggingDestination stringToLogType(const std::string &configLogType);
    std::string_view toString(LoggingSeverity logSeverity) noexcept;
    void setLogType(LoggingDestination logDst);
    void setLogFile(const std::string_view fl);
    void setSelectedLogSeverity(const std::string &logLevel);
    void setThreadPool(std::shared_ptr<threadPool> tp);
};