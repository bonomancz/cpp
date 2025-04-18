#pragma once

#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <netdb.h>
#include <unistd.h>
#include <system_error>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <net/if.h>
#include <functional>
#include <stdexcept>

class Socket {
private:
	int serverSocket = -1;
	struct addrinfo* result = nullptr;
	struct addrinfo hints;
	int serverPort = 9999;
	void initialize();
	void setup();
	void bindSock();
	void listenSock();
public:
	void build();
	int acceptSock();
	bool receiveSock(int clientSocket, std::string& receivedMessage, std::function<void(int, std::string&)> onMessageReceived, std::string& errorMessage);
	bool sendSock(int clientSocket, const std::vector<std::uint8_t>& sendMessage);
	void shutdownSock(int clientSocket);
	std::string getClientIpAddr(int clientSocket);
	void setServerPort(int serverPort);
	int getServerPort();
};

