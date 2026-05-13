#include "tariffServerMain.hpp"

int main(int argc, char** argv) {
	try {
		tariffServerMain tariffServer;
		if(tariffServer.processArguments(argc, argv)){
			tariffServer.start();
		}
	}catch (std::exception& ex) {
		std::cerr << ex.what() << std::endl;
	}
	return EXIT_SUCCESS;
}