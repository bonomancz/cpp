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
	void setThreadFinished(const std::thread::id& threadID);
	void removeThread(const std::thread::id& threadID);
	void removeAllThreads();
	void removeFinishedThreads();
	std::string getThreadPoolStatistic();
	int getThreadPoolSize();
	int getUnfinishedThreadsCount();
	std::unordered_map<std::thread::id, ThreadInfo>& getThreadPool();

	template<typename Function, typename... Args>
	void startNewThread(Function&& func, Args&&... args) {
		auto wrapper = [this, f = std::forward<Function>(func)](Args&&... innerArgs) {
			f(std::forward<Args>(innerArgs)...);
			this->setThreadFinished(std::this_thread::get_id());
			};

		std::thread newThread(wrapper, std::forward<Args>(args)...);
		{
			std::lock_guard<std::mutex> lock(threadPoolMutex);
			threadPool.emplace(
				std::piecewise_construct,
				std::forward_as_tuple(newThread.get_id()),
				std::forward_as_tuple(std::move(newThread))
			);
		}
	}
};
