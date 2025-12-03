/**
 * @file Server.hpp
 * @brief Multi-client TCP messaging server
 */

#ifndef SERVER_HPP
#define SERVER_HPP

#include "Message.hpp"
#include "ServerConfig.hpp"
#include "Dispatcher.hpp"
#include "AdminCommandHandler.hpp"
#include "CommandHandler.hpp"
#include "Utils/ThreadPool.hpp"
#include <unordered_map>
#include <unordered_set>

#include <functional>
#include <vector>
#include <string>
#include <memory>
#include <mutex>
#include <chrono>

class AdminCommandHandler;
class CommandHandler;

/**
 * @struct ClientInfo
 * @brief Complete information of a connected client
 */
struct ClientInfo {
    int socket; //File descriptor
    std::chrono::steady_clock::time_point lastPong; //Last PONG received
    bool waitingForPong = false;//Waiting for PONG
    
    ClientInfo() = default;
    explicit ClientInfo(int s) : socket(s), lastPong(std::chrono::steady_clock::now()) {}
};

/**
 * @class Server
 * @brief Messaging server with multi-client management and admin commands
 * 
 * Thread-safe. Supports heartbeat, statistics, kick, broadcast.
 */
class Server {
public:
    using CommandHandler = std::function<void(Server*, const std::vector<std::string>&, int)>;

    /**
     * @brief Default constructor
     */
    Server();
    
    /**
     * @brief Constructor with configuration
     * @param configuration Server configuration
     */
    explicit Server(const ServerConfig& configuration);
    
    /**
     * @brief Destructor
     */
    ~Server();

    /**
     * @brief Starts the server
     * @param PORT Listening port
     * @param MAX_CONNECTIONS Max connections (0 = unlimited)
     * @return 0 on success
     */
    int start(int PORT, int MAX_CONNECTIONS = 0);
    
    /**
     * @brief Stops the server and closes all connections
     */
    void stop();
    
    /**
     * @brief Executes a client command
     * @param commandName Command name
     * @param args Command arguments
     * @param socket Client socket
     */
    void executeCommand(const std::string& commandName, 
                       const std::vector<std::string>& args, 
                       int socket);
    
    /**
     * @brief Gets the server status
     * @return Current status (ON/OFF)
     */
    SERVER_STATUS getStatus() const { return status; }
    
    /**
     * @brief Gets a user's socket
     * @param username Username
     * @return Socket file descriptor (-1 if not found)
     */
    int getUserSocket(const std::string& username);
    
    /**
     * @brief Counts connected clients
     * @return Number of clients
     */
    int getClientCount() const;

    /**
     * @brief Checks if a username is already taken
     * @param username Username to check
     * @return true if already used
     */
    bool isUsernameTaken(const std::string& username);
    
    /**
     * @brief Registers a new client
     * @param username Client name
     * @param socket Client socket
     */
    void registerClient(const std::string& username, int socket);
    
    /**
     * @brief Unregisters a client
     * @param username Client name
     */
    void unregisterClient(const std::string& username);
    
    /**
     * @brief Updates a client's last pong timestamp
     * @param username Client name
     */
    void updateClientPong(const std::string& username);
    
    /**
     * @brief Gets username by socket
     * @param socket Client socket
     * @return Username (empty if not found)
     */
    std::string getUsernameBySocket(int socket);



    /**
     * @brief Gets the dispatcher
     * @return Pointer to the dispatcher
     */
    Dispatcher* getDispatcher() { return dispatcher.get(); }
    
    /**
     * @brief Increments sent messages counter
     */
    void incrementMessagesSent() { ++totalMessagesSent; }
    
    /**
     * @brief Increments received messages counter
     */
    void incrementMessagesReceived() { ++totalMessagesReceived; }
    
    /**
     * @brief Gets all connected clients
     * @return Map username -> socket
     */
    std::unordered_map<std::string, int> getAllClients();
    
    /**
     * @brief Gets the startup timestamp
     * @return Time point of startup
     */
    std::chrono::steady_clock::time_point getStartTime() const { return startTime; }
    
    /**
     * @brief Gets total received messages count
     * @return Number of received messages
     */
    size_t getTotalMessagesReceived() const { return totalMessagesReceived; }
    
    /**
     * @brief Gets total sent messages count
     * @return Number of sent messages
     */
    size_t getTotalMessagesSent() const { return totalMessagesSent; }
    
    /**
     * @brief Gets the server configuration
     * @return Current configuration
     */
    const ServerConfig& getConfig() const { return config; }

    /**
     * @brief Adds a user to the ban list
     * @param username User to add
     */
    void banlistAdd(const std::string& username);
    
    /**
     * @brief Removes a user from the ban list
     * @param username User to remove
     * @return true if the user was removed
     */
    bool banlistRemove(const std::string& username);
    
    /**
     * @brief Checks if a user is banned
     * @param username User to check
     * @return true if the user is banned
     */
    bool isBanned(const std::string& username) const;

private:
    void initializeConfig(int PORT);
    void initializeCommands();
    void createServerThreads();
    void acceptClients();
    void handleClientMessages(int clientSocket);
    
    void heartbeatLoop();
    void checkClientTimeouts();
    
    /**
     * @brief Loads banned users list from file
     */
    void loadBanlist();
    
    /**
     * @brief Saves banned users list to file
     */
    void saveBanlist();    
    
    ServerConfig config{.socket = 0, .address = {}, .port = 8080, .max_connections = 0};

    // Connected clients: username + complete info (socket + heartbeat)
    std::unordered_map<std::string, ClientInfo> clients;
    std::unordered_set<std::string> bannedUsers;
    std::unordered_map<std::string, CommandHandler> commands;

    std::unique_ptr<Dispatcher> dispatcher;
    std::unique_ptr<AdminCommandHandler> adminHandler;
    std::unique_ptr<::CommandHandler> commandHandler;
    std::unique_ptr<ThreadPool> threadPool;
    
    mutable std::mutex bannedUsersMutex;
    mutable std::mutex clientsMutex;

    size_t totalMessagesSent = 0;
    size_t totalMessagesReceived = 0;
    std::chrono::steady_clock::time_point startTime;
    SERVER_STATUS status = SERVER_STATUS::OFF;
};

#endif