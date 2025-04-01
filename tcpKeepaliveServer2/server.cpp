#include "server.h"

void Server::start() {
	try {
		while (true) {
			try {
				SOCKET clientSocket;
				std::string clientInfo;
				if ((clientSocket = sock.acceptSock()) != INVALID_SOCKET) {
					clientInfo = "Client connected: " + std::string(sock.getClientIpAddr(clientSocket));
					//std::cout << clientInfo << std::endl;
					logger(tm.getMillisecTime(), 6, clientInfo);
					this->threads.startNewThread([this, clientSocket]() {
						this->handleClient(sock, clientSocket);
					});
					this->threads.removeFinishedThreads();
				}
				else {
					std::this_thread::sleep_for(std::chrono::milliseconds(100)); // busy waiting protection
				}
				//this->status();
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

void Server::initialize() {
	logger(tm.getMillisecTime(), 6, "Starting tcpKeepaliveServer2.");
	logger(tm.getMillisecTime(), 7, "Processing parameters.");
	loadConfigParams();
	logger(tm.getMillisecTime(), 7, "Parameters processed OK.");
	sock.build();
	logger(tm.getMillisecTime(), 6, "Listening on port " + std::to_string(sock.getServerPort()) + ".");
	logger(tm.getMillisecTime(), 7, "Starting communication.");
}

void Server::stop() {
	this->threads.removeAllThreads();
}


void Server::status() {
	std::cout << "Threads in ThreadPool: " << std::to_string(this->threads.getThreadPoolSize()) << std::endl;
	std::cout << this->threads.getThreadPoolStatistic() << std::endl;
}


void Server::handleClient(Socket& sock, SOCKET clientSocket) {
	try {
		std::string clientMessage;
		std::string errorMessage;
		auto callbackHandler = [this](SOCKET &clientSocket, std::string &clientMessage) {
			this->receivedMessageHandler(clientSocket, clientMessage);
		};
		sock.receiveSock(clientSocket, clientMessage, callbackHandler, errorMessage);
		//closesocket(clientSocket);
	}
	catch (std::runtime_error& e) {
		std::cerr << "HandleClient(): Runtime error: " << e.what() << std::endl;
	}
	catch (...) {
		std::cerr << "HandleClient(): Unknown error occured." << std::endl;
	}
}


void Server::receivedMessageHandler(SOCKET &clientSocket, std::string &clientMessage) {
	try {
		std::string clientInfo;
		if (dat.isPingRequest(clientMessage)) {

			clientInfo = "(Server side log) Client requested: " + clientMessage;
			//std::cout << clientInfo << std::endl;
			logger(tm.getMillisecTime(), 6, clientInfo);

			std::string sendMessage = dat.getSendMessage(clientMessage);
			sock.sendSock(clientSocket, sendMessage);
			clientInfo = "(Server side log) Server replied: " + sendMessage;
			//std::cout << clientInfo << std::endl;
			logger(tm.getMillisecTime(), 6, clientInfo);
		}
		// set thread as finished
		std::thread::id clientThreadId = std::this_thread::get_id();
		this->threads.setThreadFinished(clientThreadId);
	}
	catch (std::runtime_error& e) {
		std::cerr << "receivedMessageHandler(): Runtime error: " << e.what() << std::endl;
	}
	catch (...) {
		std::cerr << "receivedMessageHandler(): Unknown error occured." << std::endl;
	}
}

void Server::processExecParams(int argc, char** argv) {
	std::string errorMsg;
	try {
		if (argc < 2) {
			errorMsg = "Missing arguments. Exiting due to errors.";
			logger(tm.getMillisecTime(), 3, errorMsg);
			throw std::runtime_error(errorMsg);
		}
		std::string configFile = std::string(argv[1]);
		if (configFile.find("--config=") == std::string::npos) {
			errorMsg = "Invalid arguments. Exiting due to errors.";
			logger(tm.getMillisecTime(), 3, errorMsg);
			throw std::runtime_error(errorMsg);
		}
		configFile = dat.split(configFile, '=')[1];
		setServerConfigFile(configFile);
	}
	catch (std::runtime_error& ex) {
		throw std::runtime_error(std::string(ex.what()) + "\nUsage: tcpkeepaliveserver2.exe [--config=tcpkeepaliveserver2.conf(string)]");
	}
}

void Server::loadConfigParams() {
	try {
		bool serviceFound = false;
		std::vector<std::string>redConfigLine;
		for (auto& wbConfigLine : fl.read(configFileName)) {
			std::string stripped = dat.strip(wbConfigLine, " ");
			if (wbConfigLine.empty()) { continue; }
			if (stripped[0] == '#' || stripped[0] == ';') { continue; }
			if (wbConfigLine.find("[service]") != std::string::npos) {
				serviceFound = true;
				continue;
			}
			if (serviceFound) {
				if (!wbConfigLine.empty() && wbConfigLine.find("=") != std::string::npos && wbConfigLine.find("service") != std::string::npos) {
					redConfigLine = dat.split(wbConfigLine, '=');
					if (redConfigLine.size() >= 2) {
						this->config.insert({ dat.strip(redConfigLine[0], " "), dat.strip(redConfigLine[1], " ") });
						if (redConfigLine[0].find("service_listen_port") != std::string::npos) { 
							sock.setServerPort(std::stoi(dat.strip(redConfigLine[1], " ")));
						}
						if (redConfigLine[0].find("service_log_file") != std::string::npos) {
							log.setLoggingFile(dat.strip(redConfigLine[1], " \""));
						}
					}
				}
			}
		}
		if (!serviceFound) {
			throw std::runtime_error("Server::loadConfigParams(): Exception in config file structure. Need config section [service] in config file " + configFileName);
		}
	}catch (std::exception& ex) {
		logger(tm.getMillisecTime(), 3, ex.what());
		std::cout << ex.what();
		std::exit(1);
	}
}

void Server::logger(const std::string& msecTime, const int severity, const std::string& message) {
	log.logger(msecTime, severity, message);
}


void Server::setServerConfigFile(const std::string& configFile) {
	this->configFileName = configFile;
}

void Server::setHost(std::string& host) {
	this->remoteHost = host;
	dat.setRemoteHost(host);
}

void Server::setLoopInterval(int loopInterval) {
	this->loopInterval = loopInterval;
}


void Server::setThreadsCount(int threadsCount) {
	this->threadsCount = threadsCount;
}

int Server::getThreadsCount() {
	return this->threadsCount;
}