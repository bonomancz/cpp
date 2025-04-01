#include <iostream>
#include <string>
#include <thread>
#include "client.h"

int main(int argc, char** argv) {
	Client cli;
	try {
		cli.processExecParams(argc, argv);
		cli.runInThreads();
	}
	catch (std::runtime_error& ex) {
		cli.logger(cli.getMilliSecTime(), 3, ex.what());
		std::cout << ex.what() << endl;
	}
	return 0;
}
