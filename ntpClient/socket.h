#pragma once

#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <string>
#include <vector>
#include <functional>
#include <chrono>
#include <thread>

#pragma comment (lib, "Ws2_32.lib")

using namespace std;

class Socket{
private:
	WSADATA wsaData;
	struct addrinfo* result = NULL, *ptr = NULL;
	struct addrinfo hints;
	string remoteHost;
	int remotePort;

public:
	void sockInitialize();
	bool sockConnect(SOCKET &remoteSocket);
	bool sockSend(SOCKET& remoteSocket, const std::string& data);
	bool sockSend(SOCKET &remoteSocket, const std::vector<uint8_t> &sendMessage);
	bool sockReceive(SOCKET& remoteSocket, std::vector<uint8_t> &rcvdMessage, string& errorMessage);
	void sockClose(SOCKET& remoteSocket);
	void setRemoteHost(string &host);
	void setRemotePort(int& port);
};

