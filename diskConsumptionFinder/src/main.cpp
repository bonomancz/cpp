#include <iostream>
#include "../include/diskConsumptionFinder.hpp"

int main(int argc, char **argv) {
	DiskConsumptionFinder dcf;
	try {
		dcf.run(argc, argv);
	}
	catch (std::exception& ex) {
		std::cerr << ex.what() << std::endl;
	}
	return EXIT_SUCCESS;
}