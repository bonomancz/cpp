#pragma once
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <thread>
#include <chrono>
#include <format>
#include "fileSystem.hpp"

class DiskConsumptionFinder {
private:
	std::vector<std::string> arguments;
	FileSystem fileSystem;
public:
	DiskConsumptionFinder();
	void run(int argc, char **argv);
	void processArguments(int argc, char **argv);
	std::string getConsoleHeader();
	std::string getUsage();
	std::string getBigDirs(const std::string &dirPath);
	void quit();
};
