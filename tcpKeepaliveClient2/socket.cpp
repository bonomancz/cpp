#include "socket.h"

void Socket::sockInitialize() {
	int rslt;
    if ((rslt = WSAStartup(MAKEWORD(2, 2), &wsaData)) != 0) {
        throw runtime_error("Socket::sockInitialize(): WSAStartup failed with error: " + to_string(rslt));
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // resolve remote host address and port
    if ((rslt = getaddrinfo(this->remoteHost.c_str(), to_string(remotePort).c_str(), &hints, &result)) != 0) {
        //WSACleanup();
        throw runtime_error("Socket::sockInitialize(): getaddrinfo failed with error: " + to_string(rslt));
    }
}


bool Socket::sockConnect(SOCKET &remoteSocket) {
    bool returnValue = true;
    try {
        if (result == nullptr) {
            throw std::runtime_error("Socket::SockConnect(): Exception: result is nullptr. Can not proceed.");
        }
        for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
            if ((remoteSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol)) == INVALID_SOCKET) {
                throw std::runtime_error("Socket::SockConnect(): Exception: Socket failed with error: " + to_string(WSAGetLastError()));
            }
            if (connect(remoteSocket, ptr->ai_addr, static_cast<int>(ptr->ai_addrlen)) == SOCKET_ERROR) {
                closesocket(remoteSocket);
                remoteSocket = INVALID_SOCKET;
                continue;
            }
            break;
        }
        freeaddrinfo(result);
        if (remoteSocket == INVALID_SOCKET) {
            throw std::runtime_error("Socket::SockConnect(): Exception: Unable to connect to server. INVALID_SOCKET (DST HOST:PORT).");
        }
    }
    catch (std::exception&) {
        returnValue = false;
        //std::cerr << ex.what() << std::endl;
    }
    return returnValue;
}


bool Socket::sockSend(SOCKET &remoteSocket, const string& data) {
    bool returnValue = true;
    try {
        int bytesSent;
        if ((bytesSent = send(remoteSocket, data.c_str(), static_cast<int>(data.size()), 0)) == SOCKET_ERROR) {
            throw runtime_error("Send failed with error: " + to_string(WSAGetLastError()));
        }
    }
    catch (std::exception&) {
        returnValue = false;
        //std::cerr << ex.what() << std::endl;
    }
    return returnValue;
}


bool Socket::sockReceive(SOCKET& remoteSocket, string &rcvdMessage, string& errorMessage) {
    bool returnValue = true;
    rcvdMessage.clear();
    int rslt = 0;
    const int bufferLength = 8192;
    char receiveBuffer[bufferLength];
    try {
        do {
            if ((rslt = recv(remoteSocket, receiveBuffer, bufferLength, 0)) > 0) {
                rcvdMessage.append(receiveBuffer, rslt);
                memset(&receiveBuffer, 0, bufferLength);
                if(rcvdMessage.find("PONG") != std::string::npos && rcvdMessage.size() >= 5) { // checking receive complete message
                    break;
                }
            }
            else if (rslt == 0) { // client connection closing
                throw runtime_error("Socket::receiveSock(): Exception: Server closed connection: " + to_string(WSAGetLastError()));
            }
            else {
                throw runtime_error("Socket::receiveSock(): Exception: Server data receive error: " + to_string(WSAGetLastError()));
            }
        } while (rslt > 0);
    }
    catch (std::exception& ex) {
        returnValue = false;
        errorMessage = ex.what();
    }
    return returnValue;
}

void Socket::sockClose(SOCKET &remoteSocket) {
    shutdown(remoteSocket, SD_SEND);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    closesocket(remoteSocket);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    WSACleanup();
}


void Socket::setRemoteHost(string& host) {
    this->remoteHost = host;
}

void Socket::setRemotePort(int& port) {
    this->remotePort = port;
}