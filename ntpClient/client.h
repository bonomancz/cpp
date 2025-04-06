#pragma once
#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "socket.h"
#include "data.h"
#include "time.h"
#include "threads.h"
//#include "log.h"

class Client{
private:
	Socket socket;
	Dat dat;
	Time tm;
	Threads threads;
	//Log log;
	int threadsCount;
	int remotePort;
	int loopInterval;
	std::string remoteHost;
	std::string remoteUrl;
	condition_variable cvStop, cvStart;
	mutex mtxStart, mtxStop;
	//mutex mtxLog;
public:
	void runInThreads();
	void setHost(std::string& host);
	void setPort(int port);
	void setLoopInterval(int loopInterval);
	void setThreadsCount(int threadsCount);
	int getThreadsCount();
	void processExecParams(int argc, char** argv);
	void logger(const std::string& msecTime, const int severity, const std::string& message);
	std::string getMilliSecTime();
};

