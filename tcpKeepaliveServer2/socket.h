#pragma once
#undef UNICODE
#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include <string>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <windows.h>
#include <functional>
#include <stdexcept>

#pragma comment(lib, "Ws2_32.lib")

class Socket{
private:
	WSADATA wsaData;
	SOCKET serverSocket = INVALID_SOCKET;
	struct addrinfo* result = nullptr;
	struct addrinfo hints;
	int serverPort = 5555;

	void initialize();
	void setup();
	void bindSock();
	void listenSock();
	void shutdownSock(SOCKET& clientSocket);

public:
	void build();
	SOCKET acceptSock();
	bool receiveSock(SOCKET &clientSocket, std::string &receivedMessage, std::function<void(SOCKET&, std::string&)> onMessageReceived, std::string& errorMessage);
	bool sendSock(SOCKET& clientSocket, const std::string& sendMessage);
	std::string getClientIpAddr(SOCKET &clientSocket);
	void setServerPort(int serverPort);
	int getServerPort();
};

