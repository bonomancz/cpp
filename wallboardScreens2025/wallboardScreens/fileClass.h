#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <Windows.h>

class FileManip{
private:
	std::ifstream fin;
public:
	bool fileExists(std::string name);										// checks if given file exists	
	std::string getCurrentDir();
	std::vector<std::string> read(const std::string& fileName);
	void closeFile();														// closes file
}; 
