#include "client.h"

void Client::runInThreads() {
	try {
		std::string threadLog;
		for (int i = 0; i < this->threadsCount; i++) {
			this->threads.startNewThread([this, &threadLog]() {
				std::thread::id clientThreadId = std::this_thread::get_id();
				{
					std::unique_lock<std::mutex> lockStart(mtxStart);
					cvStart.wait(lockStart, [this] { return this->threads.getThreadPoolSize() == this->getThreadsCount(); });
				}
				this->cvStart.notify_one();
				try {
					Socket sock;
					SOCKET clientSocket;
					string receivedMessage;
					sock.setRemoteHost(this->remoteHost);
					sock.setRemotePort(this->remotePort);
					bool sockConnected = false;
					string errorMessage;
					while(true) {
						if (!sockConnected) {
							sock.sockInitialize();
							if (!sock.sockConnect(clientSocket)) {
								{
									std::lock_guard<std::mutex> lockLog(mtxLog);
									log.logger(this->getMilliSecTime(), 3, "Can't connect to server.");
								}
								std::this_thread::sleep_for(std::chrono::milliseconds(this->loopInterval));
								continue;
							}
							else {
								sockConnected = true;
								{
									std::lock_guard<std::mutex> lockLog(mtxLog);
									log.logger(this->getMilliSecTime(), 6, "Connected to server.");
								}
							}
						}
						else {
							std::string sendMessage = this->dat.getSendMessage();
							//std::cout << "Sending: " + sendMessage << std::endl;
							if (sock.sockSend(clientSocket, sendMessage)) {
								{
									std::lock_guard<std::mutex> lockLog(mtxLog);
									log.logger(this->getMilliSecTime(), 6, "(Client side log) Client sent: " + sendMessage);
								}
							}
							else {
								sockConnected = false;
								{
									std::lock_guard<std::mutex> lockLog(mtxLog);
									log.logger(this->getMilliSecTime(), 3, "Client data send failed.");
								}
							}
							if (sock.sockReceive(clientSocket, receivedMessage, errorMessage)) {
								{
									std::lock_guard<std::mutex> lockLog(mtxLog);
									log.logger(this->getMilliSecTime(), 6, dat.getServerResponse(receivedMessage));
								}
							}
							else {
								sockConnected = false;
								{
									std::lock_guard<std::mutex> lockLog(mtxLog);
									log.logger(this->getMilliSecTime(), 3, "Server data receive failed...");
									if (errorMessage.find("Server closed connection") != std::string::npos) {
										log.logger(this->getMilliSecTime(), 3, "Server closed remote connection.");
									}
								}
							}
						}
						std::this_thread::sleep_for(std::chrono::milliseconds(this->loopInterval));
					}
					sock.sockClose(clientSocket);
				}
				catch (std::runtime_error &e) {
					{
						std::lock_guard<std::mutex> lockLog(mtxLog);
						threadLog += tm.getMillisecTime() + " " + e.what() + "\n";
					}
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
		cerr << "runInThreads(): Runtime error: " << e.what() << endl;
	}
}

void Client::processExecParams(int argc, char** argv) {
	logger(tm.getMillisecTime(), 6, "Starting tcpKeepaliveClient2.");
	if (argc < 5 || !dat.isInteger(argv[2]) || !dat.isInteger(argv[3]) || !dat.isInteger(argv[4])) {
		logger(tm.getMillisecTime(), 3, "Missing or invalid arguments. Exiting due to errors.");
		throw runtime_error("Missing or invalid arguments. Exiting due to errors.\nUsage: tcpkeepaliveclient2 [host (string)] [port (integer)] [threads count (integer)] [sleep ms (integer)]");
	}
	logger(tm.getMillisecTime(), 7, "Setting parameters.");
	string host = string(argv[1]);
	int port = stoi(argv[2]);
	int threadsCount = stoi(argv[3]);
	int sleepTime = stoi(argv[4]);
	setLoopInterval(sleepTime);
	setThreadsCount(threadsCount);
	setHost(host);
	setPort(port);
	logger(tm.getMillisecTime(), 7, "Parameters set OK.");
	logger(tm.getMillisecTime(), 7, "Starting communication.");
}

string Client::getMilliSecTime() {
	return tm.getMillisecTime();
}

void Client::logger(const std::string& msecTime, const int severity, const std::string& message) {
	log.logger(msecTime, severity, message);
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
