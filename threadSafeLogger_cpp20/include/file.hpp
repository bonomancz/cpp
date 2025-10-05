#pragma once
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>

class File{
public:
    File();
    void write(const std::string &fileName, const std::string &data);
    std::string read(const std::string &fileName);
};