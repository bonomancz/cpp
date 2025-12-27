#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <format>
#include <memory>
#include <thread>
#include <atomic>
#include <chrono>
#include <unordered_map>
#include <variant>
#include "chronoTime.hpp"
#include "logManager.hpp"
#include "configManager.hpp"
#include "threadManager.hpp"
#include "socketManager.hpp"
#include "sslManager.hpp"
#include "dataManager.hpp"
#include "utils.hpp"


class httpdServer{
private:
        std::atomic<bool> serverRunningFlag{true};
        std::shared_ptr<ChronoTime> timer;
        std::shared_ptr<LogManager> logger;
        std::shared_ptr<ConfigManager> config;
        std::shared_ptr<ThreadManager> threads;
        std::shared_ptr<SocketManager> sock;
        std::shared_ptr<SslManager> sslSock;
        std::shared_ptr<DataManager> datas;
        std::shared_ptr<Utils> utils;
        bool sslEnabled{true};
public:
        httpdServer();
        ~httpdServer();
        void start();
        void stop();
        void runServer();
        void processArguments(int argc, char **argv);
        void handleClient(std::shared_ptr<SocketManager> sock, SOCKET clientSocket);
        void handleClientSSL(SSL* ssl, SOCKET clientSocket);
	void receivedMessageHandler(SOCKET& clientSocket, std::string &clientMessage);
        void receivedMessageHandlerSSL(SSL* ssl, std::string &clientMessage);
        bool getServerRunningFlag() const;
        void getServerConfigParameters();
        std::string getUsage() const;
        std::string getVersion() const;
        void setSslEnabled(bool value);
};