#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

class File{
private:
	std::string fileName;
	std::ifstream file;
public:
	bool open();
	bool close();
};

