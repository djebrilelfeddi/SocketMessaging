/**
 * @file Client.hpp
 * @brief TCP messaging client
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
 * @brief Messaging client with server connection
 */
class Client {
public:
    Client(const std::string& serverAddress, int serverPort);
    ~Client();
    
    Client(const Client&) = delete;
    Client& operator=(const Client&) = delete;
    
    /**
     * @brief Connects to the server with username
     * @param username Username
     * @param outError Error message if failed (optional)
     * @return true if successful
     */
    bool connect(const std::string& username, std::string& outError);
    bool connect(const std::string& username); // Overload without error message
    
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