/**
 * @file AdminCommandHandler.hpp
 * @brief Server administrator command handler
 */

#ifndef ADMIN_COMMAND_HANDLER_HPP
#define ADMIN_COMMAND_HANDLER_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include <functional>

class Server;

/**
 * @struct AdminCommand
 * @brief Definition of an admin command
 */
struct AdminCommand {
    std::function<void(const std::vector<std::string>&)> handler;
    std::string usage;
    std::string description;
    int minArgs = 0;  // Minimum number of arguments (excluding command)
};

/**
 * @class AdminCommandHandler
 * @brief Manages the admin console and administration commands
 */
class AdminCommandHandler {
public:
    explicit AdminCommandHandler(Server* server);
    
    AdminCommandHandler(const AdminCommandHandler&) = delete;
    AdminCommandHandler& operator=(const AdminCommandHandler&) = delete;
    
    /**
     * @brief Main admin console loop (blocking)
     */
    void commandLoop();

private:
    // Commandes
    void cmdBroadcast(const std::vector<std::string>& args);
    void cmdSend(const std::vector<std::string>& args);
    void cmdList(const std::vector<std::string>& args);
    void cmdKick(const std::vector<std::string>& args);
    void cmdBan(const std::vector<std::string>& args);
    void cmdUnban(const std::vector<std::string>& args);
    void cmdStats(const std::vector<std::string>& args);
    void cmdSet(const std::vector<std::string>& args);
    void cmdConfig(const std::vector<std::string>& args);
    void cmdReset(const std::vector<std::string>& args);
    void cmdHelp(const std::vector<std::string>& args);
    void cmdStop(const std::vector<std::string>& args);
    
    // Helpers
    bool disconnectUser(const std::string& username, const std::string& reason);
    void initializeCommands();
    
    Server* server;
    std::map<std::string, AdminCommand> commands;
    bool running = true;
};

#endif