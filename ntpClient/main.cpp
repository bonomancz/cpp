#include <iostream>
#include <string>
#include "client.h"

int main(int argc, char** argv) {
	try {
		Client cli;
		cli.processExecParams(argc, argv);
		cli.runInThreads();
	}
	catch (std::exception &ex) {
		std::cerr << "Exception: " << ex.what();
	}

	return 0;
}