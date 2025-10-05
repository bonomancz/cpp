#include "../include/threadSafeLogger_cpp20.hpp"


ThreadSafeLogger::ThreadSafeLogger() : serviceStopFlag(false){}


void ThreadSafeLogger::handleQueue(std::stop_token st){
    try{
        while(!st.stop_requested()){
            std::string logMessage;
            {
                std::unique_lock<std::mutex> qlock(mtx);
                cv.wait(qlock, [this, &st](){ return !logQueue.empty() || st.stop_requested(); });
                if(st.stop_requested()){ break; }
                logMessage = logQueue.front();
                logQueue.pop();
            }
            logFl.write("./logfile.log", logMessage);
        }
    }catch(std::exception &ex){
        std::cerr << "ThreadSafeLogger::handleQueue: " << ex.what() << std::endl;
    }
}


void ThreadSafeLogger::start(){
    queueWorker = std::jthread([this](std::stop_token st){ handleQueue(st); });
}


void ThreadSafeLogger::stop(){
    queueWorker.request_stop();
    cv.notify_all();
    serviceStopFlag = true;
}


bool ThreadSafeLogger::getServiceStopFlag(){
    return serviceStopFlag;
}


void ThreadSafeLogger::log(int logDestination, const std::string &message){
    {
        std::lock_guard<std::mutex> logLock(mtx);
        logQueue.push(message);
    }
    cv.notify_one();
}