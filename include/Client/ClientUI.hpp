/**
 * @file ClientUI.hpp
 * @brief User interface for the messaging client
 */

#ifndef CLIENT_UI_HPP
#define CLIENT_UI_HPP

#include "Client/Client.hpp"
#include "Client/MessageHandler.hpp"
#include <string>
#include <functional>
#include <unordered_map>
#include <atomic>
#include <mutex>
#include <queue>

/**
 * @class ClientUI
 * @brief Manages all user interface and display
 */
class ClientUI {
public:
    ClientUI(Client& client, const std::string& serverIp, int serverPort);
    
    /**
     * @brief Launches the user interface (connection + main loop)
     * @return true if normal exit, false if error
     */
    bool run();
    
    // Callback for server events (called by listening thread)
    void onServerEvent(const ServerEventData& event);

private:
    using Command = std::function<void()>;
    
    // Connection
    bool promptAndConnect();
    
    // Display
    void clearScreen();
    void printHeader();
    void printMenu();
    void print(const std::string& msg);
    void printError(const std::string& msg);
    void printSuccess(const std::string& msg);
    void promptAndWait();
    void printUserList(const std::string& userListData);
    
    // Menu commands
    void cmdSendMessage(bool broadcast);
    void cmdListUnread();
    void cmdReadMessage();
    void cmdListUsers();
    void cmdGetLog();
    
    // Server events management
    void displayEvent(const ServerEventData& event);
    void displayPendingEvents();
    void waitForResponse(ServerEvent expectedType, int timeoutMs = 3000);
    
    Client& client;
    std::string serverIp;
    int serverPort;
    std::unordered_map<std::string, Command> commands;
    
    std::atomic<bool> inCommand{false};
    std::mutex eventQueueMutex;
    std::queue<ServerEventData> pendingEvents;
    ServerEvent lastReceivedEvent{ServerEvent::NONE};
};

#endif
