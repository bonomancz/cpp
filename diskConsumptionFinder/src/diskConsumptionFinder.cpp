#include "../include/diskConsumptionFinder.hpp"


DiskConsumptionFinder::DiskConsumptionFinder() {}


void DiskConsumptionFinder::run(int argc, char **argv) {
	std::cout << getConsoleHeader() << std::endl;
	processArguments(argc, argv);

	std::string userSelection;
	bool driveSelected = false;
	bool folderSelected = false;
	bool firstTimeFolderSelected = false;
	int userDriveSelected;
	int userFolderSelected;
	std::string searchPath;

	while(true){
		try {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			std::ostringstream userMenu;
			userMenu << std::endl;
			std::vector<std::string> diskDrives = fileSystem.getAvailableDiskDrives();
			if (driveSelected == false) {

				userMenu << "Available disk drives: " << std::endl;

				int counter = 1;
				for (const auto& item : diskDrives) {
					userMenu << std::format("[{:>2}] Disk [ {} ]: {}", counter, item, fileSystem.getDriveSize(item)) << std::endl;
					counter++;
				}

				userMenu << std::endl;
				userMenu << "Select disk drive number, 0 for disk selection or 'q' for exit [1]: ";
				std::cout << userMenu.str();
				userMenu.str("");
				userMenu.clear();
				std::getline(std::cin, userSelection);
				if (userSelection == "q") {
					quit();
				}
				else {
					driveSelected = true;
					if (userSelection == "0") {
						driveSelected = false;
						continue;
					}
					if (userSelection.empty()) {
						userSelection = "1";
					}
					userDriveSelected = std::stoi(userSelection);
					if (userDriveSelected > 0) {
						userDriveSelected -= 1;
						userMenu << std::endl << std::format("Selected disk [ {} ]: {}", diskDrives[userDriveSelected], fileSystem.getDriveSize(diskDrives[userDriveSelected])) << std::endl;
						std::cout << userMenu.str();
						userMenu.str("");
						userMenu.clear();
					}
				}
			}

			userMenu << std::endl;

			std::string userMenuStr;

			if (driveSelected && folderSelected) {
				std::cout << "Analyzing selected folder directory structure...";
				std::string searchFolderPath;
				if (firstTimeFolderSelected) {
					searchFolderPath = searchPath;
				}
				else {
					firstTimeFolderSelected = true;
					searchFolderPath = diskDrives[userDriveSelected];
				}
				searchPath = fileSystem.getMaxItemsSizeSorted(fileSystem.getDirectoryMaxSizeItems(searchFolderPath))[userFolderSelected].first;
				if (!fileSystem.isDir(searchPath)) {
					std::cout << "Selected item MUST BE a directory : " << searchPath << std::endl;
					folderSelected = false;
					continue;
				}
			}
			else {
				std::cout << "Analyzing disk directory structure..." << std::endl;
				searchPath = diskDrives[userDriveSelected];
			}
			userMenuStr = getBigDirs(searchPath);

			userMenu << userMenuStr;
			userMenu << std::endl;
			userMenu << "Select folder number, 0 for disk selection or 'q' for exit [1]: ";
			std::cout << userMenu.str();
			userMenu.str("");
			userMenu.clear();
			std::getline(std::cin, userSelection);

			if (userSelection == "q") {
				quit();
			}
			else {
				folderSelected = true;
				if (userSelection == "0") {
					driveSelected = false;
					folderSelected = false;
					continue;
				}
				if (userSelection.empty()) {
					userSelection = "1";
				}
				userFolderSelected = std::stoi(userSelection);
				if (userFolderSelected > 0) {
					userFolderSelected -= 1;
					userMenu << std::endl;
					std::cout << userMenu.str();
					userMenu.str("");
					userMenu.clear();
				}
			}
		}catch (std::exception& ex) {
				std::cerr << "run(): " << ex.what() << std::endl;
		}
	}
}


std::string DiskConsumptionFinder::getConsoleHeader() {
	std::string output = "Disk consumption finder, version 1.12, Windows (2025), Jan Novotny <bonoman@volny.cz> (Free to use for everyone)";
	return output;
}


std::string DiskConsumptionFinder::getUsage() {
	std::string output = "Usage: diskConsumptionFinder.exe [disk letter: (C:\\)]";
	return output;
}


void DiskConsumptionFinder::processArguments(int argc, char **argv) {
	arguments.assign(argv, argv + argc);
	if (arguments.size() < 1) {
		throw std::runtime_error("Error: Missing drive letter followed by colon or absolute directory path.");
	}
}


std::string DiskConsumptionFinder::getBigDirs(const std::string& dirPath) {
	std::ostringstream sortedValues;
	int dirCount = 20;
	std::vector<std::pair<std::string, std::uintmax_t>> dirVec = fileSystem.getMaxItemsSizeSorted(fileSystem.getDirectoryMaxSizeItems(dirPath));
	int strLength = 0;
	for (size_t i = 0; i < dirVec.size() && i < dirCount; i++) {
		if (dirVec[i].first.size() > strLength) {
			strLength = dirVec[i].first.size();
		}
	}
	strLength += 10;
	int modifiedStrLength = strLength;
	std::string dirSign;

	for (size_t i = 0; i < dirVec.size() && i < dirCount; i++) {
		dirSign = "";
		strLength = modifiedStrLength;
		if (fileSystem.isDir(dirVec[i].first)) {
			dirSign = "[DIR] ";
			strLength -= 6;
		}
		sortedValues << std::format("[{:>2}] ({}{:<{}}) {:>15.3f} GB", i+1, dirSign, dirVec[i].first, strLength, fileSystem.getGbSize(dirVec[i].second)) << std::endl;
	}	
	return sortedValues.str();
}


void DiskConsumptionFinder::quit() {
	exit(EXIT_SUCCESS);
}
