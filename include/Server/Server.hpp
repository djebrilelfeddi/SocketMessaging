/**
 * @file Server.hpp
 * @brief Serveur TCP de messagerie multi-clients
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
 * @brief Informations complètes d'un client connecté
 */
struct ClientInfo {
    int socket; //File descriptor
    std::chrono::steady_clock::time_point lastPong; //Dernier PONG reçu
    bool waitingForPong = false;//En attente de PONG
    
    ClientInfo() = default;
    explicit ClientInfo(int s) : socket(s), lastPong(std::chrono::steady_clock::now()) {}
};

/**
 * @class Server
 * @brief Serveur de messagerie avec gestion multi-clients et commandes admin
 * 
 * Thread-safe. Supporte heartbeat, statistiques, kick, broadcast.
 */
class Server {
public:
    using CommandHandler = std::function<void(Server*, const std::vector<std::string>&, int)>;

    /**
     * @brief Constructeur par défaut
     */
    Server();
    
    /**
     * @brief Constructeur avec configuration
     * @param configuration Configuration du serveur
     */
    explicit Server(const ServerConfig& configuration);
    
    /**
     * @brief Destructeur
     */
    ~Server();

    /**
     * @brief Démarre le serveur
     * @param PORT Port d'écoute
     * @param MAX_CONNECTIONS Nombre max de connexions (0 = illimité)
     * @return 0 si succès
     */
    int start(int PORT, int MAX_CONNECTIONS = 0);
    
    /**
     * @brief Arrête le serveur et ferme toutes les connexions
     */
    void stop();
    
    /**
     * @brief Exécute une commande client
     * @param commandName Nom de la commande
     * @param args Arguments de la commande
     * @param socket Socket du client
     */
    void executeCommand(const std::string& commandName, 
                       const std::vector<std::string>& args, 
                       int socket);
    
    /**
     * @brief Obtient le statut du serveur
     * @return Statut actuel (ON/OFF)
     */
    SERVER_STATUS getStatus() const { return status; }
    
    /**
     * @brief Récupère le socket d'un utilisateur
     * @param username Nom d'utilisateur
     * @return File descriptor du socket (-1 si non trouvé)
     */
    int getUserSocket(const std::string& username);
    
    /**
     * @brief Compte les clients connectés
     * @return Nombre de clients
     */
    int getClientCount() const;

    /**
     * @brief Vérifie si un nom d'utilisateur est déjà pris
     * @param username Nom à vérifier
     * @return true si déjà utilisé
     */
    bool isUsernameTaken(const std::string& username);
    
    /**
     * @brief Enregistre un nouveau client
     * @param username Nom du client
     * @param socket Socket du client
     */
    void registerClient(const std::string& username, int socket);
    
    /**
     * @brief Désenregistre un client
     * @param username Nom du client
     */
    void unregisterClient(const std::string& username);
    
    /**
     * @brief Met à jour le timestamp de dernier pong d'un client
     * @param username Nom du client
     */
    void updateClientPong(const std::string& username);
    
    /**
     * @brief Obtient le nom d'utilisateur par socket
     * @param socket Socket du client
     * @return Nom d'utilisateur (vide si non trouvé)
     */
    std::string getUsernameBySocket(int socket);



    /**
     * @brief Obtient le dispatcher
     * @return Pointeur vers le dispatcher
     */
    Dispatcher* getDispatcher() { return dispatcher.get(); }
    
    /**
     * @brief Incrémente le compteur de messages envoyés
     */
    void incrementMessagesSent() { ++totalMessagesSent; }
    
    /**
     * @brief Incrémente le compteur de messages reçus
     */
    void incrementMessagesReceived() { ++totalMessagesReceived; }
    
    /**
     * @brief Obtient tous les clients connectés
     * @return Map username -> socket
     */
    std::unordered_map<std::string, int> getAllClients();
    
    /**
     * @brief Obtient le timestamp de démarrage
     * @return Point dans le temps du démarrage
     */
    std::chrono::steady_clock::time_point getStartTime() const { return startTime; }
    
    /**
     * @brief Obtient le nombre total de messages reçus
     * @return Nombre de messages reçus
     */
    size_t getTotalMessagesReceived() const { return totalMessagesReceived; }
    
    /**
     * @brief Obtient le nombre total de messages envoyés
     * @return Nombre de messages envoyés
     */
    size_t getTotalMessagesSent() const { return totalMessagesSent; }
    
    /**
     * @brief Obtient la configuration du serveur
     * @return Configuration actuelle
     */
    const ServerConfig& getConfig() const { return config; }

    /**
     * @brief Ajoute un utilisateur à la liste de bannissement
     * @param username Utilisateur à ajouter
     */
    void banlistAdd(const std::string& username);
    
    /**
     * @brief Retire un utilisateur de la liste de bannissement
     * @param username Utilisateur à retirer
     * @return true si l'utilisateur a été retiré
     */
    bool banlistRemove(const std::string& username);
    
    /**
     * @brief Vérifie si un utilisateur est banni
     * @param username Utilisateur à vérifier
     * @return true si l'utilisateur est banni
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
     * @brief Charge la liste des utilisateurs bannis depuis le fichier
     */
    void loadBanlist();
    
    /**
     * @brief Sauvegarde la liste des utilisateurs bannis dans le fichier
     */
    void saveBanlist();    
    
    ServerConfig config{.socket = 0, .address = {}, .port = 8080, .max_connections = 0};

    // Clients connectés : username + infos complètes (socket + heartbeat)
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