#pragma once
#include <string>
#include <sstream>
#include <fstream>
#include <thread>
#include <chrono>
#include <vector>
#include <mutex>
#include <unordered_map>
#include "socket.h"
#include "data.h"
#include "threads.h"
#include "time.h"
#include "file.h"
#include "log.h"

class Server{
private:
	Socket sock;
	Data dat;
	Threads threads;
	Time tm;
	File fl;
	Log log;
	std::string configFileName;
	std::unordered_map<std::string, std::string> config;
	int threadsCount;
	int loopInterval;
	std::string remoteHost;

public:
	void initialize();
	void start();
	void stop();
	void status();
	void handleClient(Socket& sock, SOCKET clientSocket);
	void receivedMessageHandler(SOCKET& clientSocket, std::string& clientMessage);
	void processExecParams(int argc, char** argv);
	void setServerConfigFile(const std::string& configFile);
	void loadConfigParams();
	void logger(const std::string& msecTime, const int severity, const std::string& message);
	void setHost(std::string& host);
	void setLoopInterval(int loopInterval);
	void setThreadsCount(int threadsCount);
	int getThreadsCount();
};


