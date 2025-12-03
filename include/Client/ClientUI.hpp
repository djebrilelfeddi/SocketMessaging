/**
 * @file ClientUI.hpp
 * @brief Interface utilisateur pour le client de messagerie
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
 * @brief Gère toute l'interface utilisateur et l'affichage
 */
class ClientUI {
public:
    ClientUI(Client& client, const std::string& serverIp, int serverPort);
    
    /**
     * @brief Lance l'interface utilisateur (connexion + boucle principale)
     * @return true si sortie normale, false si erreur
     */
    bool run();
    
    // Callback pour les événements serveur (appelé par le thread d'écoute)
    void onServerEvent(const ServerEventData& event);

private:
    using Command = std::function<void()>;
    
    // Connexion
    bool promptAndConnect();
    
    // Affichage
    void clearScreen();
    void printHeader();
    void printMenu();
    void print(const std::string& msg);
    void printError(const std::string& msg);
    void printSuccess(const std::string& msg);
    void promptAndWait();
    void printUserList(const std::string& userListData);
    
    // Commandes du menu
    void cmdSendMessage(bool broadcast);
    void cmdListUnread();
    void cmdReadMessage();
    void cmdListUsers();
    void cmdGetLog();
    
    // Gestion des événements serveur
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
