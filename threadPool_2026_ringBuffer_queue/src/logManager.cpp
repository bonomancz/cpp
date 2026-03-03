#include "../include/logManager.hpp"
#include "../include/chronoTime.hpp"


LogManager::LogManager(ChronoTime &tmr) : timer(tmr){
    debug("LogManager initialized.");
    debug("Starting Logmanager.");
    start();
}


LogManager::~LogManager(){ stop(); }


void LogManager::processLogQueue(){
    try{
        std::string lgMsg;
        {
            std::unique_lock<std::mutex> lock(logQueueMtx);
            logQueueCv.wait(lock, [this](){ return !circularLogQueue.isEmpty() || !logManagerRunningFlag; });
            if(circularLogQueue.isEmpty()){ return; }
            lgMsg = std::move(circularLogQueue.dequeue());
        }
        switch(selectedLogType){
            case LoggingDestination::Off:       break;
            case LoggingDestination::Stdout:    std::cout << lgMsg << std::endl;
                                                break;
            case LoggingDestination::File:      break;
            case LoggingDestination::Syslog:    break;
            case LoggingDestination::Database:  break;
            case LoggingDestination::Gui:       break;
            default:                            break;
        }
    }catch(std::exception &ex){
        std::string exMsg = std::format("LogManager::processLogQueue(): {}", ex.what());
        std::cerr << exMsg << std::endl;
    }
}


void LogManager::pushLogQueue(const LoggingSeverity logSeverity, const std::string_view logMessage){
    try{
        if(logSeverity < selectedLogSeverity){ return;}
        std::string formattedLogMsg = std::format("{:<37}{}", std::format("{} [{}]:", timer.getCurrentTimeMs(), toString(logSeverity)), logMessage);
        {
            std::lock_guard<std::mutex> lgLock(logQueueMtx);
            if(circularLogQueue.isFull()){ return; }
            circularLogQueue.enqueue(std::move(formattedLogMsg));
        }
        logQueueCv.notify_one();   
    }catch(std::exception &ex){
        std::string exMsg = std::format("LogManager::pushLogQueue(): {}", ex.what());
        std::cerr << exMsg << std::endl;
    }
}


void LogManager::trace(const std::string_view logMessage){ pushLogQueue(LoggingSeverity::Trace, logMessage); }
void LogManager::debug(const std::string_view logMessage){ pushLogQueue(LoggingSeverity::Debug, logMessage); }
void LogManager::info(const std::string_view logMessage){ pushLogQueue(LoggingSeverity::Info, logMessage); }
void LogManager::notice(const std::string_view logMessage){ pushLogQueue(LoggingSeverity::Notice, logMessage); }
void LogManager::warning(const std::string_view logMessage){ pushLogQueue(LoggingSeverity::Warning, logMessage); }
void LogManager::error(const std::string_view logMessage){ pushLogQueue(LoggingSeverity::Error, logMessage); }
void LogManager::critical(const std::string_view logMessage){ pushLogQueue(LoggingSeverity::Critical, logMessage); }
void LogManager::alert(const std::string_view logMessage){ pushLogQueue(LoggingSeverity::Alert, logMessage); }
void LogManager::emergency(const std::string_view logMessage){ pushLogQueue(LoggingSeverity::Emergency, logMessage); }


std::string_view LogManager::toString(LoggingSeverity logSeverity) noexcept{
    switch(logSeverity){
        case LoggingSeverity::Trace:    return "TRACE";
        case LoggingSeverity::Debug:    return "DEBUG";
        case LoggingSeverity::Info:     return "INFO";
        case LoggingSeverity::Notice:   return "NOTICE";
        case LoggingSeverity::Warning:  return "WARNING";
        case LoggingSeverity::Error:    return "ERROR";
        case LoggingSeverity::Critical: return "CRITICAL";
        case LoggingSeverity::Alert:    return "ALERT";
        case LoggingSeverity::Emergency:return "EMERGENCY";
        default:                        return "UNKNOWN";
    }
}


LoggingSeverity LogManager::stringToSeverity(const std::string &configSeverity){
    std::string lower = configSeverity;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    if(auto it = severityMap.find(lower); it != severityMap.end()){
        return it->second;
    }
    throw std::invalid_argument("LogManager::stringToSeverity(): Unknown severity level: " + configSeverity);
}


void LogManager::start(){
    try{
        trace("LogManager started.");
        logQueueWorker = std::jthread([this](std::stop_token stt){
            while(!stt.stop_requested() && logManagerRunningFlag){
                processLogQueue();
            }
            debug("LogManager thread stopped.");
        });
    }catch(std::exception &ex){
        std::string exMsg = std::format("LogManager::start(): {}", ex.what());
        std::cerr << exMsg << std::endl;
        error(exMsg);
    }
}


void LogManager::stop(){
    try{
        debug("LogManager Stopping.");
        logManagerRunningFlag = false;
        // logQueue.setDone();
        logQueueWorker.request_stop();
    }catch(std::exception &ex){
        std::string exMsg = std::format("LogManager::stop(): {}", ex.what());
        std::cerr << exMsg << std::endl;
        error(exMsg);
    }    
}


void LogManager::setLogType(LoggingDestination logDst){ selectedLogType = logDst; }
void LogManager::setLogFile(const std::string_view logFl){ logFile = logFl; }
void LogManager::setSelectedLogSeverity(const std::string &logLevel){ selectedLogSeverity = stringToSeverity(logLevel); }
