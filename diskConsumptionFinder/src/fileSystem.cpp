#include "../include/fileSystem.hpp"

FileSystem::FileSystem() {}
FileSystem::~FileSystem() {}


bool FileSystem::isDiskDrive(const std::string& drivePath) {
	bool result = false;
	try {
		std::filesystem::path fsPath(drivePath);
		if (std::filesystem::exists(fsPath)) {
			result = true;
		}
	}
	catch (std::exception& ex) {
		std::cerr << "isDiskDrive(): " << ex.what() << std::endl;
	}
	return result;
}


std::vector<std::string> FileSystem::getAvailableDiskDrives() {
	std::vector<std::string> output;
	for (char drive = 'A'; drive <= 'Z'; drive++) {
		std::string driveRoot = std::string(1, drive) + ":\\";
		if (isDiskDrive(driveRoot)) {
			output.emplace_back(driveRoot);
		}
	}
	return output;
}


uintmax_t FileSystem::getFsObjSizeBytes(const std::string& drivePath) {
	std::filesystem::path fsPath(drivePath);
	uintmax_t size = 0;
	try {
		if (std::filesystem::exists(fsPath)) {
			std::error_code ec;

			if (std::filesystem::is_regular_file(fsPath, ec)) {
				auto fileSize = std::filesystem::file_size(fsPath, ec);
				if (ec) {
					ec.clear();
					return 0;
				}
				return fileSize;
			}

			for (const auto& entry : std::filesystem::recursive_directory_iterator(fsPath, std::filesystem::directory_options::skip_permission_denied, ec)) {
				if (ec) {
					//std::cerr << "[Error] " << entry.path().string() << " - " << ec.message() << std::endl;
					ec.clear();
					continue;
				}
				if (entry.is_regular_file(ec)) {
					auto fileSize = entry.file_size(ec);
					if (ec) {
						//std::cerr << "[ERROR] " << entry.path() << " - " << ec.message() << std::endl;
						ec.clear();
					}
					else {
						if (fileSize == 0) {
							//std::cerr << "[ZERO SIZE] " << entry.path() << std::endl;
						}
						size += fileSize;
					}
				}
				ec.clear();
			}
		}
	}
	catch (const std::exception& ex) {
		std::cerr << "FileSystem::getFsObjSizeBytes(): " << ex.what() << std::endl;
	}
	return size;
}


float FileSystem::getFsObjSizeGb(const std::string& drivePath) {
	float size = 0.0;
	try {
		size = static_cast<float>(getFsObjSizeBytes(drivePath)) / (1024 * 1024 * 1024);
	}
	catch (const std::exception& ex) {
		std::cout << "FileSystem::getFsObjSizeGb(): " << ex.what() << std::endl;
	}
	return size;
}


std::string FileSystem::getDriveSize(const std::string& drivePath) {
	std::string spaceAvailable = "N/A";
	std::filesystem::path fsPath(drivePath);
	if (std::filesystem::exists(fsPath)) {
		auto space = std::filesystem::space(fsPath);

		float spcAvlbl = static_cast<float>(space.available) / (1024 * 1024 * 1024);
		float drvSz = static_cast<float>(space.capacity) / (1024 * 1024 * 1024);
		float prcAvlbl = 100.f * static_cast<float>(space.available) / static_cast<float>(space.capacity);

		spaceAvailable = std::format("Space available: {:>8.1f} GB ({:.1f} %)", spcAvlbl, prcAvlbl);
		std::string driveSize = std::format("Drive capacity: {} GB", drvSz);
	}
	return spaceAvailable;
}


std::unordered_map<std::string, uintmax_t> FileSystem::getDirectoryMaxSizeItems(const std::string& drivePath) {
	std::unordered_map<std::string, uintmax_t> resultMap;
	std::filesystem::path fsPath(drivePath);
	try {
		if (std::filesystem::exists(fsPath)) {
			std::error_code ec;
			int counter = 1;
			for (const auto& entry : std::filesystem::directory_iterator(fsPath, std::filesystem::directory_options::skip_permission_denied, ec)) {
				if (ec) {
					ec.clear();
					continue;
				}
				//std::cout << std::format("[{:>6}] [{:<80}] {:>20} bytes", counter, entry.path().string(), getFsObjSizeBytes(entry.path().string())) << std::endl;
				resultMap.try_emplace(entry.path().string(), getFsObjSizeBytes(entry.path().string()));
				ec.clear();
				counter++;
			}
		}
	}
	catch (const std::exception &ex) {
		std::cerr << "FileSystem::getDirectoryMaxSizeItems(): " << ex.what();
	}
	return resultMap;
}


std::vector<std::pair<std::string, uintmax_t>> FileSystem::getMaxItemsSizeSorted(const std::unordered_map<std::string, uintmax_t>& dirsMap) {
	std::vector<std::pair<std::string, uintmax_t>> items(dirsMap.begin(), dirsMap.end());
	std::sort(items.begin(), items.end(), [](const auto& a, const auto& b) {
		return a.second > b.second;
	});
	return items;
}


float FileSystem::getGbSize(uintmax_t objSize) {
	return objSize > 0 ? 1.0f * objSize / (1024 * 1024 * 1024) : 0.0;
}


bool FileSystem::isFile(const std::string& objPath) {
	std::filesystem::path fsPath(objPath);
	if (std::filesystem::exists(fsPath)) {
		std::error_code ec;
		if (std::filesystem::is_regular_file(fsPath, ec)) {
			return true;
		}
	}
	return false;
}


bool FileSystem::isDir(const std::string& objPath) {
	std::filesystem::path fsPath(objPath);
	if (std::filesystem::exists(fsPath)) {
		std::error_code ec;
		if (std::filesystem::is_directory(fsPath, ec)) {
			return true;
		}
	}
	return false;
}
