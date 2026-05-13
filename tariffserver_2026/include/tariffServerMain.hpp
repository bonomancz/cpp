#pragma once
#include <iostream>
#include <string>
#include <string_view>
#include <sstream>
#include <format>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <stop_token>
#include <chrono>
#include "./tariffSentenceData.hpp"
#include "./utils.hpp"
#include "./chronoTime.hpp"
#include "./logManager.hpp"
#include "./threadPool.hpp"
#include "./socketManager.hpp"
#include "./databaseManager.hpp"
#include "./EpollManager.hpp"
#include "./fileManager.hpp"
#include "./configManager.hpp"


class tariffServerMain {
private:
        int threadCount;
        std::shared_ptr<FileManager> file;
        std::shared_ptr<tariffSentenceData> tData;
        std::shared_ptr<ChronoTime> timer;
        std::shared_ptr<Utils> utils;
        std::shared_ptr<LogManager> logger;
        std::unique_ptr<ConfigManager> config;
        std::shared_ptr<threadPool> threads;
        std::shared_ptr<EpollManager> epoll;
        std::unique_ptr<SocketManager> sock;
        std::unique_ptr<databaseManager> db;
        std::condition_variable serverCv;
        std::mutex serverMtx;
        std::stop_source serverStopSource;
        std::jthread tasksThread, monitoringThread, sqlDbFlushThread;
        std::atomic<bool> tariffServerRunningFlag{true};
        void runServer(std::stop_token stopToken);
        void sendTestTasksToThreadPool();
        void startMonitoringThread();
        void startSqlDbFlushThread();
        void shutdown();
        void timedOutShutdown(int secondsTimeout);
        void receivedMessageHandler(int clientSocket, const std::string_view receivedMessage);
        std::string getVersion() const;
        std::string getUsage() const;
        void setConfigFileValues();

public:
        tariffServerMain();
        ~tariffServerMain();
        void start();
        void stop();
        bool processArguments(int argc, char **argv);
};
