#pragma once
#include <iostream>
#include <unordered_map>
#include <atomic>
#include <thread>
#include <mutex>
#include <sstream>

struct ThreadInfo {
	std::thread threadObj;
	std::atomic<bool> isFinished;

	ThreadInfo(std::thread&& t) : threadObj(std::move(t)), isFinished(false) {};
	// default constructor
	ThreadInfo() : isFinished(false) {}

	// deny copy
	ThreadInfo(const ThreadInfo&) = delete;
	ThreadInfo& operator=(const ThreadInfo&) = delete;

	// allow transfer
	ThreadInfo(ThreadInfo&&) noexcept = default;
	ThreadInfo& operator=(ThreadInfo&&) noexcept = default;
};


class Threads {
private:
	std::unordered_map<std::thread::id, ThreadInfo> threadPool;
	std::mutex threadPoolMutex;

public:
	void setThreadFinished(std::thread::id& threadID);
	void removeThread(std::thread::id& threadID);
	void removeAllThreads();
	void removeFinishedThreads();
	std::string getThreadPoolStatistic();
	int getThreadPoolSize();
	std::unordered_map<std::thread::id, ThreadInfo>& getThreadPool();

	template<typename Function, typename... Args>
	void startNewThread(Function&& func, Args&&... args) {
		std::thread thread1(std::forward<Function>(func), std::forward<Args>(args)...);
		{
			std::lock_guard<std::mutex> lock(threadPoolMutex);
			this->threadPool.emplace(thread1.get_id(), std::move(thread1));
		}
	}
};

