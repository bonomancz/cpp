#include "socket.h"

void Socket::build() {
    this->initialize();
    this->setup();
    this->bindSock();
    this->listenSock();
}

void Socket::initialize() {
    int rslt = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (rslt != 0) {
        throw std::runtime_error("Socket::initialize(): WSAStartup failed with error: " + std::to_string(rslt));
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // resolve address, port
    rslt = getaddrinfo(NULL, std::to_string(serverPort).c_str(), &hints, &result);
    if (rslt != 0) {
        //WSACleanup();
        throw std::runtime_error("Socket::initialize(): getaddrinfo failed with error: " + std::to_string(rslt));
    }
}

void Socket::setup() {
    this->serverSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (this->serverSocket == INVALID_SOCKET) {
        freeaddrinfo(result);
        //WSACleanup();
        throw std::runtime_error("Socket::setup(): Socket failed with error: " + std::to_string(WSAGetLastError()));
    }
}

void Socket::bindSock() {
    int rslt = ::bind(this->serverSocket, result->ai_addr, (int)result->ai_addrlen);
    freeaddrinfo(result);
    if (rslt == SOCKET_ERROR) {
        closesocket(this->serverSocket);
        //WSACleanup();
        throw std::runtime_error("Socket::bindSock(): Bind failed with error: " + std::to_string(WSAGetLastError()));
    }
}

void Socket::listenSock() {
    int rslt = listen(this->serverSocket, SOMAXCONN);
    if (rslt == SOCKET_ERROR) {
        closesocket(this->serverSocket);
        //WSACleanup();
        throw std::runtime_error("Socket::listenSock(): Listen failed with error: " + std::to_string(WSAGetLastError()));
    }
}

SOCKET Socket::acceptSock() {
    SOCKET output = accept(serverSocket, NULL, NULL);
    if (output == INVALID_SOCKET) {
        closesocket(serverSocket);
        //WSACleanup();
        throw std::runtime_error("Socket::acceptSock(): Accept failed with error: " + std::to_string(WSAGetLastError()));
    }
    return output;
}

bool Socket::receiveSock(SOCKET &clientSocket, std::string &rcvdMessage, std::function<void(SOCKET&, std::string&)> onMessageReceived, std::string &errorMessage) {
    bool returnValue = true;
    rcvdMessage.clear();
    int rslt = 0;
    const int bufferLength = 8192;
    char receiveBuffer[bufferLength];

    try {
        // max wait time 1sec for client data else close client connection
        fd_set readFds;
        FD_ZERO(&readFds);
        FD_SET(clientSocket, &readFds);
        timeval timeout;
        timeout.tv_sec = 3;
        timeout.tv_usec = 0;
        int selectResult = select(0, &readFds, NULL, NULL, &timeout);
        if (selectResult == 0) {
            throw std::runtime_error("Socket::receiveSock(): Client timeout: Client connected but sent no data in 3sec. Closing client connection.: " + std::to_string(WSAGetLastError()));
        }
        else if (selectResult < 0) {
            throw std::runtime_error("Socket::receiveSock(): Socket select(): Error: " + std::to_string(WSAGetLastError()));
        }

        do {
            if ((rslt = recv(clientSocket, receiveBuffer, bufferLength, 0)) > 0) {
                rcvdMessage.append(receiveBuffer, rslt);
                memset(&receiveBuffer, 0, bufferLength);
                if (rcvdMessage.find("PING") != std::string::npos && rcvdMessage.size() >= 5) { // checking receive complete message
                    onMessageReceived(clientSocket, rcvdMessage); // solution with callback call
                    rcvdMessage.clear();
                }
                else {
                    throw std::runtime_error("Socket::receiveSock(): Invalid client request received. Closing client connection.");
                }
            }
            else if (rslt == 0) { // client connection closing
                throw std::runtime_error("Socket::receiveSock(): Exception: Client closed connection: " + std::to_string(WSAGetLastError()));
            }
            else {
                throw std::runtime_error("Socket::receiveSock(): Exception: Client data receive error: " + std::to_string(WSAGetLastError()));
            }
        } while (rslt > 0);
    }
    catch (std::exception& ex) {
        returnValue = false;
        errorMessage = ex.what();
    }
    return returnValue;
}

bool Socket::sendSock(SOCKET& clientSocket, const std::string& sendMessage) {
    bool returnValue = true;
    try {
        int bytesSent;
        if ((bytesSent = send(clientSocket, sendMessage.c_str(), static_cast<int>(sendMessage.size()), 0)) == SOCKET_ERROR) {
            throw std::runtime_error("Send failed with error: " + std::to_string(WSAGetLastError()));
        }
    }
    catch (std::exception&) {
        returnValue = false;
        //std::cerr << ex.what() << std::endl;
    }
    return returnValue;
}

void Socket::shutdownSock(SOCKET &clientSocket) {
    int rslt = shutdown(clientSocket, SD_SEND);
    if (rslt == SOCKET_ERROR) {
        closesocket(clientSocket);
        //WSACleanup();
        throw std::runtime_error("Socket::shutdownSock(): Shutdown failed with error: " + std::to_string(WSAGetLastError()));
    }
}

std::string Socket::getClientIpAddr(SOCKET& clientSocket) {
    std::string output = "";
    sockaddr_in clientIpAddr;
    int addrSize = sizeof(clientIpAddr);
    char clientIP[INET_ADDRSTRLEN];
    if (getpeername(clientSocket, (sockaddr*)&clientIpAddr, &addrSize) == 0) {
        inet_ntop(AF_INET, &clientIpAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
        output = std::string(clientIP);
    }
    return output;
}

void Socket::setServerPort(int serverPort) { this->serverPort = serverPort; }
int Socket::getServerPort() { return this->serverPort; }
