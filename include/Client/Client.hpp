/**
 * @file Client.hpp
 * @brief Client TCP de messagerie
 */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "MessageHandler.hpp"
#include "Utils/Socket.hpp"
#include <string>
#include <memory>
#include <thread>
#include <functional>

/**
 * @class Client
 * @brief Client de messagerie avec connexion au serveur
 */
class Client {
public:
    Client(const std::string& serverAddress, int serverPort);
    ~Client();
    
    Client(const Client&) = delete;
    Client& operator=(const Client&) = delete;
    
    /**
     * @brief Connecte au serveur avec le nom d'utilisateur
     * @param username Nom d'utilisateur
     * @param outError Message d'erreur si échec (optionnel)
     * @return true si succès
     */
    bool connect(const std::string& username, std::string& outError);
    bool connect(const std::string& username); // Surcharge sans message d'erreur
    
    void disconnect();
    void startListening(MessageHandler::EventCallback onEvent);
    
    MessageHandler* getMessageHandler() const;
    bool connected() const { return isConnected; }
    std::string getCurrentUsername() const { return username; }
    
private:
    std::string serverAddress;
    int serverPort;
    Socket clientSocket;
    bool isConnected = false;
    std::string username;
    std::unique_ptr<MessageHandler> messageHandler;
    std::thread listenerThread;
};

#endif