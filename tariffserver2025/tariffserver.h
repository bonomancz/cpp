#pragma once
#include <iostream>
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
#include "config.h"
#include "utils.h"

class Tariffserver{
private:
	Socket sock;
	Data dat;
	Config conf;
	Threads threads;
	Time tm;
	Log log;
	std::unordered_map<std::string, std::string> config;

public:
	void initialize();
	void start();
	void stop();
	void status();
	void handleClient(Socket& sock, int clientSocket);
	void receivedMessageHandler(int clientSocket, std::string& clientMessage);
	void processExecParams(int argc, char** argv);
	void logger(const std::string& msecTime, const int severity, const std::string& message);
};

