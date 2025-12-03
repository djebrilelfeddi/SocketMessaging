/**
 * @file MessageHandler.hpp
 * @brief Message handler for the client
 */

#ifndef MESSAGING_HANDLER_HPP
#define MESSAGING_HANDLER_HPP

#include <string>
#include <vector>
#include <functional>
#include <mutex>
#include <chrono>
#include <atomic>
#include <optional>

/**
 * @struct ReceivedMessage
 * @brief Represents a message received by the client
 */
struct ReceivedMessage {
    std::string from;
    std::string subject;
    std::string body;
    std::chrono::system_clock::time_point timestamp;
    std::chrono::system_clock::time_point receivedAt;
    bool isRead = false;
    int index = 0;
};

/**
 * @brief Types of events received from the server
 */
enum class ServerEvent {
    NONE,
    MESSAGE,
    OK,
    ERROR_MSG,
    USERS,
    LOG,
    PING
};

/**
 * @brief Data of a server event
 */
struct ServerEventData {
    ServerEvent type;
    std::string data;
    std::vector<std::string> args;
};

/**
 * @class MessageHandler
 * @brief Manages messages and network communication (without display)
 */
class MessageHandler {
public:
    using EventCallback = std::function<void(const ServerEventData&)>;
    
    explicit MessageHandler(int socketFd);
    
    // Command sending
    bool sendMessage(const std::string& to, const std::string& subject, const std::string& body);
    bool sendCommand(const std::string& command);
    
    // Listen (blocking)
    void listen(EventCallback onEvent);
    
    // Stored messages management
    std::vector<ReceivedMessage> getUnreadMessages() const;
    int getUnreadCount() const;
    bool readMessageByIndex(int index, ReceivedMessage& out);
    bool replyToMessage(int originalIndex, const std::string& body);
    
    // Accesseurs
    int getSocketFd() const { return socketFd; }
    void setCurrentUsername(const std::string& u) { currentUsername = u; }
    std::string getCurrentUsername() const { return currentUsername; }

private:
    std::optional<ServerEventData> parseMessage(const std::string& raw);
    void storeMessage(const ServerEventData& data);
    
    int socketFd;
    std::string currentUsername;
    std::vector<ReceivedMessage> unreadMessages;
    std::vector<ReceivedMessage> readMessages;
    int messageCounter = 0;
    mutable std::mutex messagesMutex;
};

#endif