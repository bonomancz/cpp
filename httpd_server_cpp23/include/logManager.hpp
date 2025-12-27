#pragma once
#include <iostream>
#include <string>
#include <format>
#include <queue>
#include <atomic>
#include <memory>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <unordered_map>
#include "threadSafeQueue.hpp"


class ChronoTime;

enum class LoggingDestination { Off, Stdout, File, Syslog, Database };

enum class LoggingSeverity { Trace, Debug, Info, Notice, Warning, Error, Critical, Alert, Emergency };

class LogManager{
private:
    LoggingDestination selectedLogType = LoggingDestination::Stdout;
    LoggingSeverity selectedLogSeverity = LoggingSeverity::Trace;
    std::shared_ptr<ChronoTime> timer;
    std::string logFile = "./defaultserver.log";
    std::jthread logQueueWorker;
    std::mutex logQueueMtx;
    std::condition_variable logQueueCv;
    std::atomic<bool> logManagerRunningFlag{true};
    threadSafeQueue<std::string> logQueue;
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
public:
    LogManager(std::shared_ptr<ChronoTime> chrTime);
    ~LogManager();
    void start();
    void stop();
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
    void processLogQueue();
    LoggingSeverity stringToSeverity(const std::string &configSeverity);
    std::string_view toString(LoggingSeverity logSeverity) noexcept;
    void setLogType(LoggingDestination logDst);
    void setLogFile(const std::string_view fl);
    void setSelectedLogSeverity(const std::string &logLevel);
};