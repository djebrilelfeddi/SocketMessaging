/**
 * @file MessageHandler.hpp
 * @brief Gestionnaire de messages pour le client
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
 * @brief Représente un message reçu par le client
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
 * @brief Types d'événements reçus du serveur
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
 * @brief Données d'un événement serveur
 */
struct ServerEventData {
    ServerEvent type;
    std::string data;
    std::vector<std::string> args;
};

/**
 * @class MessageHandler
 * @brief Gère les messages et la communication réseau (sans affichage)
 */
class MessageHandler {
public:
    using EventCallback = std::function<void(const ServerEventData&)>;
    
    explicit MessageHandler(int socketFd);
    
    // Envoi de commandes
    bool sendMessage(const std::string& to, const std::string& subject, const std::string& body);
    bool sendCommand(const std::string& command);
    
    // Écoute (bloquant)
    void listen(EventCallback onEvent);
    
    // Gestion des messages stockés
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