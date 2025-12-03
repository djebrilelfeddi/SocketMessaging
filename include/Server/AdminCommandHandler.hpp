/**
 * @file AdminCommandHandler.hpp
 * @brief Gestionnaire de commandes administrateur du serveur
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
 * @brief Définition d'une commande admin
 */
struct AdminCommand {
    std::function<void(const std::vector<std::string>&)> handler;
    std::string usage;
    std::string description;
    int minArgs = 0;  // Nombre minimum d'arguments (hors commande)
};

/**
 * @class AdminCommandHandler
 * @brief Gère la console admin et les commandes d'administration
 */
class AdminCommandHandler {
public:
    explicit AdminCommandHandler(Server* server);
    
    AdminCommandHandler(const AdminCommandHandler&) = delete;
    AdminCommandHandler& operator=(const AdminCommandHandler&) = delete;
    
    /**
     * @brief Boucle principale de la console admin (bloquante)
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