#include "Utils/NetworkStream.hpp"
#include "Utils/Logger.hpp"
#include "Utils/Constants.hpp"
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>

namespace Network {

NetworkStream::NetworkStream(int socket) 
    : socketFd(socket)
    , connected(socket >= 0) {
}

bool NetworkStream::send(const std::string& message) {
    if (!connected) {
        return false;
    }
    
    std::string encoded = encodeMessage(message);
    ssize_t bytesSent = ::send(socketFd, encoded.c_str(), encoded.length(), MSG_NOSIGNAL);
    
    if (bytesSent < 0) {
        connected = false;
        return false;
    }
    
    return true;
}

std::optional<std::string> NetworkStream::receive() {
    if (!connected) {
        return std::nullopt;
    }
    
    bool success;
    std::string message = decodeMessage(success);
    
    if (!success) {
        connected = false;
        return std::nullopt;
    }
    
    return message;
}

bool NetworkStream::isConnected() const {
    return connected;
}

std::string NetworkStream::encodeMessage(const std::string& message) {
    uint32_t length = static_cast<uint32_t>(message.length());
    uint32_t networkLength = htonl(length);
    
    std::string encoded;
    encoded.reserve(sizeof(networkLength) + length);
    
    encoded.append(reinterpret_cast<const char*>(&networkLength), sizeof(networkLength));
    encoded.append(message);
    
    return encoded;
}

std::string NetworkStream::decodeMessage(bool& success) {
    success = false;
    
    uint32_t networkLength;
    ssize_t bytesRead = recv(socketFd, &networkLength, sizeof(networkLength), MSG_WAITALL);
    
    if (bytesRead != sizeof(networkLength)) {
        return "";
    }
    
    uint32_t length = ntohl(networkLength);
    
    if (length == 0 || length > Constants::MAX_MESSAGE_SIZE) {
        return "";
    }
    
    std::string message(length, '\0');
    bytesRead = recv(socketFd, &message[0], length, MSG_WAITALL);
    
    if (bytesRead != static_cast<ssize_t>(length)) {
        return "";
    }
    
    success = true;
    return message;
}

}
