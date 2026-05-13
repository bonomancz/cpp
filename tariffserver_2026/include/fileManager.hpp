#pragma once

#include <iostream>
#include <filesystem>
#include <string>
#include <fstream>
#include <vector>
#include <mutex>


class FileManager {
private:
	std::mutex fileMutex;
public:
	void write(const std::string& fileName, const std::string& data);
	void write(const std::string& fileName, const std::string& data, bool append);
	std::vector<std::string> read(const std::string& fileName);
	bool fileExists(std::string &configFile);
};
