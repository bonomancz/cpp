#include <iostream>
#include <string>
#include "tariffserver.h"

int main(int argc, char** argv) {
	try {
		Tariffserver tariffserver;
		tariffserver.processExecParams(argc, argv);
		tariffserver.initialize();
		tariffserver.start();
	}
	catch (std::exception &ex) {
		std::cerr << ex.what() << std::endl;
	}
	return 0;
}