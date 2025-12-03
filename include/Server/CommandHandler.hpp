/**
 * @file CommandHandler.hpp
 * @brief Client command handler for the server
 */

#ifndef COMMAND_HANDLER_HPP
#define COMMAND_HANDLER_HPP

#include <vector>
#include <string>

class Server;

/**
 * @class CommandHandler
 * @brief Processes commands received from clients
 * 
 * Handles CONNECT, DISCONNECT, SEND, PING, PONG, LIST_USERS, GET_LOG
 */
class CommandHandler {
public:
    /**
     * @brief Constructor
     * @param server Pointer to the server
     */
    explicit CommandHandler(Server* server);

    /**
     * @brief Handles client connection
     * @param parsedData Parsed data [CONNECT, username]
     * @param socket Client socket
     */
    void handleConnect(const std::vector<std::string>& parsedData, int socket);
    
    /**
     * @brief Handles client disconnection
     * @param parsedData Parsed data [DISCONNECT]
     * @param socket Client socket
     */
    void handleDisconnect(const std::vector<std::string>& parsedData, int socket);
    
    /**
     * @brief Handles message sending
     * @param parsedData Parsed data [SEND, to, subject, body, timestamp]
     * @param socket Sender socket
     */
    void handleSendMessage(const std::vector<std::string>& parsedData, int socket);
    
    /**
     * @brief Handles a ping
     * @param parsedData Parsed data [PING]
     * @param socket Client socket
     */
    void handlePing(const std::vector<std::string>& parsedData, int socket);
    
    /**
     * @brief Handles a pong (heartbeat response)
     * @param parsedData Parsed data [PONG]
     * @param socket Client socket
     */
    void handlePong(const std::vector<std::string>& parsedData, int socket);
    
    /**
     * @brief Handles user list request
     * @param parsedData Parsed data [LIST_USERS]
     * @param socket Client socket
     */
    void handleListUsers(const std::vector<std::string>& parsedData, int socket);
    
    /**
     * @brief Handles log request
     * @param parsedData Parsed data [GET_LOG]
     * @param socket Client socket
     */
    void handleGetLog(const std::vector<std::string>& parsedData, int socket);    CommandHandler(const CommandHandler&) = delete;
    CommandHandler& operator=(const CommandHandler&) = delete;

private:
    /**
     * @brief Sends a raw response to the client
     * @param socket Client socket
     * @param message Message to send
     */
    void sendResponse(int socket, const std::string& message);
    
    /**
     * @brief Sends an OK response to the client
     * @param socket Client socket
     * @param message Optional message (empty by default)
     */
    void sendOK(int socket, const std::string& message = "");
    
    /**
     * @brief Sends an ERROR response to the client
     * @param socket Client socket
     * @param error Error message
     */
    void sendError(int socket, const std::string& error);
    
    Server* server;
};

#endif
