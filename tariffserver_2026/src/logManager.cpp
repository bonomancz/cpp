#include "../include/logManager.hpp"
#include "../include/fileManager.hpp"
#include "../include/chronoTime.hpp"
#include "../include/threadPool.hpp"


LogManager::LogManager(std::shared_ptr<ChronoTime> tmr, std::shared_ptr<FileManager> fl) : timer(tmr), file(fl){
    debug("LogManager initialized.");
    debug("Starting Logmanager.");
}


LogManager::~LogManager(){}


void LogManager::processLogQueue(const std::string &logMessage){
    try{       
        switch(selectedLogType){
            case LoggingDestination::Off:       break;
            case LoggingDestination::Stdout:    std::cout << logMessage << std::endl;
                                                break;
            case LoggingDestination::File:      file->write(logFile, logMessage, true);
                                                break;
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
        if(logSeverity < selectedLogSeverity){ return; }
        std::string formattedLogMsg = std::format("{:<37}{}", std::format("{} [{}]:", timer->getCurrentTimeMs(), toString(logSeverity)), logMessage);

        if(pool && pool->isRunning()){
            pool->threadPoolSubmitTask(workerRole::LOG, [this, msg = std::move(formattedLogMsg)](){
                this->processLogQueue(std::move(msg));
            });
        }else{
            std::cout << formattedLogMsg << std::endl;
        }
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


LoggingDestination LogManager::stringToLogType(const std::string &configLogType){
    std::string lower = configLogType;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    if(auto it = logTypeMap.find(lower); it != logTypeMap.end()){
        return it->second;
    }
    throw std::invalid_argument("LogManager::stringToLogType(): Unknown log type: " + configLogType);
}


void LogManager::setLogType(LoggingDestination logDst){ selectedLogType = logDst; }
void LogManager::setLogFile(const std::string_view logFl){ logFile = logFl; }
void LogManager::setSelectedLogSeverity(const std::string &logLevel){ selectedLogSeverity = stringToSeverity(logLevel); }
void LogManager::setThreadPool(std::shared_ptr<threadPool> tp){ pool = tp; }
