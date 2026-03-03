#pragma once
#include <iostream>
#include <string>
#include <format>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <stop_token>
#include <chrono>
#include "./utils.hpp"
#include "./chronoTime.hpp"
#include "./logManager.hpp"
#include "./threadPool.hpp"

class threadPoolMain{
private:
        int threadCount;
        int taskWeight;
        Utils utils;
        ChronoTime timer, &refTimer;
        LogManager logger, &refLogger;
        threadPool threads;
        std::condition_variable mainLoopCv;
        std::mutex mainLoopMtx;
        std::stop_source mainLoopStopSource;
        std::jthread tasksThread, monitoringThread;
public:
        threadPoolMain();
        ~threadPoolMain();
        void runMainServerLoop(std::stop_token stopToken);
        void start();
        void stop();
        void shutdown();
        void timedOutShutdown(int secondsTimeout);
        void sendTestTasksToThreadPool();
        void startMonitoringThread();
        void heavyTask();
        int64_t fibTask(int64_t n);
};
