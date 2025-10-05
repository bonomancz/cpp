#pragma once
#include <iostream>
#include <sstream>
#include <thread>
#include <string>
#include <future>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <chrono>
#include "./file.hpp"

class ThreadSafeLogger{
private:
    std::queue<std::string> logQueue;
    std::mutex mtx;
    std::jthread queueWorker;
    std::condition_variable cv;
    bool serviceStopFlag;
    File logFl;
public:
    ThreadSafeLogger();
    void handleQueue(std::stop_token ts);
    void log(int logDestination, const std::string &message);
    void start();
    void stop();
    bool getServiceStopFlag();
};