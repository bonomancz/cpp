#include "fileManager.hpp"

std::vector<std::string> FileManager::read(const std::string& fileName) {
	try {
		std::vector<std::string> read_content;
		std::ifstream fin(fileName);
		if (!fin) {
			throw std::runtime_error("File::read(): Exception: Failed open file. (" + fileName + ")");
		}
		std::string line;
		while (getline(fin, line)) {
			read_content.emplace_back(line);
		}
		return read_content;
	}
	catch (const std::exception& ex) {
		std::cerr << ex.what() << std::endl;
	}
	return {};
}

void FileManager::write(const std::string& fileName, const std::string& data, bool append) {
	std::lock_guard<std::mutex> lock(fileMutex);
	try {
		std::ofstream fout(fileName, append ? std::ios::app : std::ios::trunc);
		if (!fout) {
			throw std::runtime_error("File::write(): Exception: Failed open file. (" + fileName + ")");
		}
		fout << data << std::endl;
		if (!fout.good()) {
			throw std::runtime_error("File::write(): Exception: writing to file failed. (" + fileName + ")");
		}
	}
	catch (const std::exception& ex) {
		std::cerr << ex.what() << std::endl;
	}
}

void FileManager::write(const std::string& fileName, const std::string& data) {
	std::lock_guard<std::mutex> lock(fileMutex);
	write(fileName, data, false);
}

bool FileManager::fileExists(std::string& configFile){
	return std::filesystem::exists(configFile);
}