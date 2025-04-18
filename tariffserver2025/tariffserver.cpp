#include "tariffserver.h"
#include "utils.h"


void Tariffserver::start() {
	try {
		while (true) {
			try {
				int clientSocket;
				std::string clientInfo;
				if ((clientSocket = sock.acceptSock()) != -1) {
					clientInfo = "Client connected: " + std::string(sock.getClientIpAddr(clientSocket));
					logger(tm.getMillisecTime(), 6, clientInfo);
					this->threads.startNewThread([this, clientSocket]() {
						this->handleClient(sock, clientSocket);
						});
					this->threads.removeFinishedThreads();
				}
				else {
					std::this_thread::sleep_for(std::chrono::milliseconds(100));
				}
				this->status();
			}
			catch (std::runtime_error& e) {
				std::cerr << "main(): Runtime error: " << e.what() << std::endl;
			}
		}
	}
	catch (std::runtime_error& e) {
		std::cerr << "main(): Runtime error: " << e.what() << std::endl;
	}
	catch (...) {
		std::cerr << "main(): Unknown error occured." << std::endl;
	}
}


void Tariffserver::initialize() {
	try {
		logger(tm.getMillisecTime(), 6, "Starting tcpKeepaliveServer2.");
		logger(tm.getMillisecTime(), 7, "Processing parameters.");
		conf.buildServiceConfigMap();
		int listenPort = conf.get_listenPort();
		logger(tm.getMillisecTime(), 7, "Parameters processed OK.");
		sock.setServerPort(listenPort);
		sock.build();
		logger(tm.getMillisecTime(), 6, "Listening on port " + std::to_string(sock.getServerPort()) + ".");
		logger(tm.getMillisecTime(), 7, "Server started.");

		// start data storage (database) handler thread
		std::string storageInfoMsg = "Datastorage is unavailable.";
		if (dat.isDataStorageInitialized(conf)) {
			storageInfoMsg = "Datastorage available ok.";
			this->threads.startNewThread([this]() {
				logger(tm.getMillisecTime(), 7, "Database handler thread started.");
				this->dat.processBufferToStorage();
				});
		}
		logger(tm.getMillisecTime(), 6, storageInfoMsg);
	}
	catch (std::runtime_error& ex) {
		logger(tm.getMillisecTime(), 2, std::string(ex.what()));
		throw std::runtime_error(std::string(ex.what()));
	}
}


void Tariffserver::stop() {
	this->threads.removeAllThreads();
}


void Tariffserver::status() {
	logger(tm.getMillisecTime(), 7, "Active threads: " + std::to_string(this->threads.getThreadPoolSize()) + ".");
}


void Tariffserver::handleClient(Socket& sock, int clientSocket) {
	try {
		std::string clientMessage;
		std::string errorMessage;
		auto callbackHandler = [this](int clientSocket, std::string& clientMessage) {
			this->receivedMessageHandler(clientSocket, clientMessage);
			};

		std::thread::id clientThreadId = std::this_thread::get_id();
		logger(tm.getMillisecTime(), 7, "Receive started (Thread: " + Utils::threadIdToString(clientThreadId) + ").");
		sock.receiveSock(clientSocket, clientMessage, callbackHandler, errorMessage);
	}
	catch (std::exception& e) {
		logger(tm.getMillisecTime(), 7, "HandleClient(): " + std::string(e.what()) + ".");
	}
	catch (...) {
		logger(tm.getMillisecTime(), 7, "HandleClient(): Unknown error occured.");
	}

	try {
		sock.shutdownSock(clientSocket);
	}
	catch (const std::exception& ex) {
		std::cerr << "handleClient(): shutdownSock() failed: " + std::string(ex.what()) << std::endl;
	}
	// set thread as finished
	std::thread::id clientThreadId = std::this_thread::get_id();
	logger(tm.getMillisecTime(), 7, "Receive finished (Thread: " + Utils::threadIdToString(clientThreadId) + ").");
	this->threads.setThreadFinished(clientThreadId);
}


void Tariffserver::receivedMessageHandler(int clientSocket, std::string& clientMessage) {
	try {
		dat.processRcvdMessage(clientMessage);
	}
	catch (std::exception& e) {
		std::cerr << "receivedMessageHandler(): Exception: " << e.what() << std::endl;
	}
	catch (...) {
		std::cerr << "receivedMessageHandler(): Unknown error occured." << std::endl;
	}
}

void Tariffserver::processExecParams(int argc, char** argv) {
	try {
		conf.processExecParams(argc, argv);
	}
	catch (std::runtime_error& ex) {
		throw std::runtime_error(std::string(std::string(ex.what())));
	}
}

void Tariffserver::logger(const std::string& msecTime, const int severity, const std::string& message) {
	log.logger(msecTime, severity, message);
}
