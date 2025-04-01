#include <iostream>
#include <string>
#include "server.h"

int main(int argc, char** argv) {
	try {
		Server srv;
		srv.processExecParams(argc, argv);
		srv.initialize();
		srv.start();
	}
	catch(std::runtime_error &e){
		std::cerr << "tcpkeepaliveserver2: " << e.what();
	}
	catch(...){
		std::cerr << "tcpkeepaliveserver2: Unknown error occured.";
	}

	return 0;
}
