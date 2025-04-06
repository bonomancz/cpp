#include "client.h"

void Client::runInThreads() {
	try {
		for (int i = 0; i < this->threadsCount; i++) {
			this->threads.startNewThread([this]() {
				std::thread::id clientThreadId = std::this_thread::get_id();
				{
					std::unique_lock<std::mutex> lockStart(mtxStart);
					cvStart.wait(lockStart, [this] { return this->threads.getThreadPoolSize() == this->getThreadsCount(); });
				}
				this->cvStart.notify_one();
				try {
					Socket sock;
					SOCKET clientSocket;
					std::vector<uint8_t> receivedMessage;
					sock.setRemoteHost(this->remoteHost);
					sock.setRemotePort(this->remotePort);
					bool sockConnected = false;
					string errorMessage;
					while(true) {
						if (!sockConnected) { // not for UDP (TCP only)
							sock.sockInitialize();
							if (!sock.sockConnect(clientSocket)) {
								std::cerr << tm.getMillisecTime() << " Can't connect to server." << std::endl;
								std::this_thread::sleep_for(std::chrono::milliseconds(this->loopInterval));
								continue;
							}
							else {
								sockConnected = true;
								//std::cout << tm.getMillisecTime() << " Directed to server." << std::endl;
							}
						}
						if(sockConnected) {
							std::vector<uint8_t> sendMessage = this->dat.getSendMessage();
							if (sock.sockSend(clientSocket, sendMessage)) {
								std::cout << tm.getMillisecTime() << " Client request sent" << std::endl;
							}
							else {
								sockConnected = false;
								std::cout << tm.getMillisecTime() << " Client data sent failed" << std::endl;
							}
							if (sock.sockReceive(clientSocket, receivedMessage, errorMessage)) {
								std::cout << tm.getMillisecTime() << " " << dat.getServerResponse(receivedMessage) << std::endl;
							}
							else {
								sockConnected = false;
								std::cerr << tm.getMillisecTime() << " Server data receive failed..." << std::endl;
								if (errorMessage.find("Server closed connection") != std::string::npos) {
									std::cerr << tm.getMillisecTime() << " Remote server closed connection." << std::endl;
								}

							}
						}
						std::this_thread::sleep_for(std::chrono::milliseconds(this->loopInterval));
					}
					sock.sockClose(clientSocket);
				}
				catch (std::runtime_error &e) {
					std::cerr << tm.getMillisecTime() << " Server data receive failed... " << e.what() << std::endl;
				}
				this->threads.setThreadFinished(clientThreadId);
				this->cvStop.notify_one();
			});
		}

		{
			std::unique_lock <std::mutex> lockStop(mtxStop);
			cvStop.wait(lockStop, [this] { return this->threads.getUnfinishedThreadsCount() == 0; });
		}
		this->threads.removeFinishedThreads();
	}
	catch (std::runtime_error &e) {
		cerr << tm.getMillisecTime() << " runInThreads(): Runtime error: " << e.what() << endl;
	}
}

void Client::processExecParams(int argc, char** argv) {
	if (argc < 5 || !dat.isInteger(argv[2]) || !dat.isInteger(argv[3]) || !dat.isInteger(argv[4])) {
		throw runtime_error("Missing or invalid arguments. Exiting due to errors.\nUsage: ntpClient.exe [host (string)] [port (integer)] [threads count (integer)] [loop interval ms (integer)]");
	}
	string host = string(argv[1]);
	int port = stoi(argv[2]);
	int threadsCount = stoi(argv[3]);
	int sleepTime = stoi(argv[4]);
	setLoopInterval(sleepTime);
	setThreadsCount(threadsCount);
	setHost(host);
	setPort(port);
}

string Client::getMilliSecTime() {
	return tm.getMillisecTime();
}

void Client::setHost(std::string& host) {
	this->remoteHost = host;
	dat.setRemoteHost(host);
}

void Client::setPort(int port) {
	this->remotePort = port;
}

void Client::setLoopInterval(int loopInterval) {
	this->loopInterval = loopInterval;
}


void Client::setThreadsCount(int threadsCount) {
	this->threadsCount = threadsCount;
}

int Client::getThreadsCount() {
	return this->threadsCount;
}
