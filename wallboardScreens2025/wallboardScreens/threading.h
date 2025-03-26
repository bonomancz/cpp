#include <iostream>
#include <chrono>
#include <vector>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <Windows.h>

class Threads{
private:
	std::vector<std::string> threadCommands;
	std::vector<std::thread> threadPool;
	std::condition_variable startCV;
	std::mutex startMutex;
	bool readyToStart = false;
public:
	void buildParams(const std::vector<std::string>& wbs, const std::string& path);
	void threadMsWorkerWithTimeout(const std::string& command, DWORD msTimeout);
	void runThreadPoolWithTimeout(DWORD msTimeout);
};
