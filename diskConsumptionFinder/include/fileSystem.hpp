#pragma once
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <format>
#include <filesystem>
#include <unordered_map>

class FileSystem {
private:
public:
	FileSystem();
	~FileSystem();
	bool isDiskDrive(const std::string& drivePath);
	bool isFile(const std::string& objPath);
	bool isDir(const std::string& objPath);
	std::vector<std::string> getAvailableDiskDrives();
	uintmax_t getFsObjSizeBytes(const std::string& drivePath);
	float getFsObjSizeGb(const std::string& drivePath);
	std::string getDriveSize(const std::string& drivePath);
	std::unordered_map<std::string, uintmax_t> getDirectoryMaxSizeItems(const std::string& drivePath);
	std::vector<std::pair<std::string, uintmax_t>> getMaxItemsSizeSorted(const std::unordered_map<std::string, uintmax_t> &dirsMap);
	float getGbSize(uintmax_t objSize);
};