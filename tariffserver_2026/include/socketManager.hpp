#pragma once

#include <iostream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <memory>
#include <functional>
#include <vector>
#include <format>
#include <cstring>
#include <cerrno>
#include <chrono>
#include <thread>
#include <atomic>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <netinet/in.h>


struct clientContext {
    int fd;
    std::string ipAddress;
    std::vector<uint8_t> buffer;
    std::chrono::steady_clock::time_point lastActivity;
};

class LogManager;
class EpollManager;
using DataReceivedCallback = std::function<void(int, std::string_view)>;

class SocketManager{
private:
	std::shared_ptr<LogManager> logger;
	std::shared_ptr<EpollManager> epoll;
	struct addrinfo* result{nullptr};
    struct addrinfo hints{};
	int serverPort;
    int serverSocket;
	std::unordered_map<int, std::unique_ptr<clientContext>> connectedClients;
	std::atomic<bool> socketManagerRunningFlag{true};
	DataReceivedCallback onClientDataReceivedCallback;

	void initialize();
	void setup();
	void bindSock();
	void listenSock();
	void shutdownSock(int clientSocket);

public:
	SocketManager(std::shared_ptr<LogManager> lgMgr, std::shared_ptr<EpollManager> epMgr);
	~SocketManager();
	void build();
	// int acceptSock();
	std::vector<int> acceptSock();
	void registerClient(int clientSocket);
	void removeClient(int clientSocket);
	// void receiveSock(int clientSocket, std::string &receivedMessage, const std::function<void(int, std::string_view)> onMessageReceived);
	// void sendSock(int clientSocket, const std::string &sendMessage);
	std::string getClientIpAddr(int clientSocket);
	std::string getServerHostname();
	int getServerSocket() const;	
	void setServerPort(int serverPort);
	void networkEventLoop();
	void onClientDataReady(clientContext* ctx);
	void setOnClientDataReceivedCallback(std::function<void(int, std::string_view)> callback);
	clientContext* getClientContext(int clientSocket);
	void start();
	void stop();
};

