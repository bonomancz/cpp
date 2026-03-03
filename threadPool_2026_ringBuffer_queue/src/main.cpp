#include "threadPoolMain.hpp"

int main(int argc, char** argv) {
	try {
		threadPoolMain threads;		
		threads.start();
	}catch (std::exception& ex) {
		std::cerr << ex.what() << std::endl;
	}
	return EXIT_SUCCESS;
}