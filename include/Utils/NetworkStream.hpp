/**
 * @file NetworkStream.hpp
 * @brief Network abstraction with automatic encoding/decoding
 */

#ifndef NETWORK_STREAM_HPP
#define NETWORK_STREAM_HPP

#include <string>
#include <optional>

namespace Network {

/**
 * @class NetworkStream
 * @brief Encapsulates send/receive with [4-byte length][data] protocol
 * 
 * Automatically handles encoding (length prefix) and decoding.
 * Non-copyable.
 */
class NetworkStream {
public:
    /**
     * @brief Constructor
     * @param socket Connected socket file descriptor
     */
    explicit NetworkStream(int socket);
    
    NetworkStream(const NetworkStream&) = delete;
    NetworkStream& operator=(const NetworkStream&) = delete;
    
    /**
     * @brief Sends a message (automatic encoding)
     * @param message Data to send
     * @return true if send succeeded
     */
    [[nodiscard]] bool send(const std::string& message);
    
    /**
     * @brief Receives a message (automatic decoding)
     * @return Received message or std::nullopt on failure
     */
    [[nodiscard]] std::optional<std::string> receive();
    
    /**
     * @brief Checks if connection is active
     * @return true if connected
     */
    [[nodiscard]] bool isConnected() const;
    
    /**
     * @brief Obtient le file descriptor
     * @return Socket fd
     */
    [[nodiscard]] int getSocket() const { return socketFd; }
    
private:
    std::string encodeMessage(const std::string& message);
    std::string decodeMessage(bool& success);
    
    int socketFd;
    bool connected;
};

} // namespace Network

#endif