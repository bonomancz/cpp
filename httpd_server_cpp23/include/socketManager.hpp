#undef UNICODE
#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include <string>
#include <memory>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <windows.h>
#include <functional>
#include <format>

#pragma once

class LogManager;

class SocketManager{
private:
	std::shared_ptr<LogManager> logger;
	WSADATA wsaData;
	SOCKET serverSocket = INVALID_SOCKET;
	struct addrinfo* result = nullptr;
	struct addrinfo hints;
	int serverPort = 443;

	void initializeWinSock2API();
	void initialize();
	void setup();
	void bindSock();
	void listenSock();
	void shutdownSock(SOCKET& clientSocket);

public:
	SocketManager(std::shared_ptr<LogManager> lgMgr);
	~SocketManager();
	void initWSA();
	void build();
	SOCKET acceptSock();
	void receiveSock(SOCKET &clientSocket, std::string &receivedMessage, const std::function<void(SOCKET&, std::string&)>& onMessageReceived);
	void sendSock(SOCKET &clientSocket, std::string &sendMessage);
	std::string getClientIpAddr(SOCKET &clientSocket);
	std::string getServerHostname();
	void setServerPort(int serverPort);
};

