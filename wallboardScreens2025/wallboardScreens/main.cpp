#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include "config.h"
#include "threading.h"

using namespace std;

int main(int argc, char *argv[]){
	try{
		Config config;
		Threads threads;

		// getting config parameters
		config.getWallboardParams(argc, argv);
		std::string browserPath = config.get_Service_browser_path();
		int loopTimeout = config.get_Service_loop_timeout();
		int threadTimeout = config.get_Service_thread_timeout();

		threads.buildParams(config.getConfiguredWallboards(), browserPath);
		// handling threads endless loop
		while(true){
			threads.runThreadPoolWithTimeout(threadTimeout);
			std::this_thread::sleep_for(std::chrono::milliseconds(loopTimeout));
			//cout << "Main thread running..." << endl;
		}
	}catch(std::exception &e){                                    
		std::cout << "\nwallboardScreens: error -- " << string(e.what()) << endl;
		exit(0);
	}
	
	return 0;
}