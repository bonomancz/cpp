#include "fileClass.h"

bool FileManip::fileExists(std::string name) {
	bool retVal = true;
	std::ifstream file(name);
	if (!file) {
		return false;
	}
	return retVal;
}

std::string FileManip::getCurrentDir() {
	const unsigned long maxDir = 260;
	char currentDir[maxDir];
	GetCurrentDirectory(maxDir, currentDir);
	return std::string(currentDir);
}

std::vector<std::string> FileManip::read(const std::string& fileName) {
	try {
		std::vector<std::string> read_content;
		std::ifstream fin(fileName);
		if (!fin) {
			throw std::runtime_error("File::read(): Exception: open ifstream failed. (" + fileName + ")");
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

// closes file
void FileManip::closeFile() {
	if (fin.is_open()) {
		fin.close();
	}
}
