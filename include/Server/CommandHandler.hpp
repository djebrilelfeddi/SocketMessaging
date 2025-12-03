/**
 * @file CommandHandler.hpp
 * @brief Gestionnaire des commandes clients du serveur
 */

#ifndef COMMAND_HANDLER_HPP
#define COMMAND_HANDLER_HPP

#include <vector>
#include <string>

class Server;

/**
 * @class CommandHandler
 * @brief Traite les commandes reçues des clients
 * 
 * Gère CONNECT, DISCONNECT, SEND, PING, PONG, LIST_USERS, GET_LOG
 */
class CommandHandler {
public:
    /**
     * @brief Constructeur
     * @param server Pointeur vers le serveur
     */
    explicit CommandHandler(Server* server);

    /**
     * @brief Gère la connexion d'un client
     * @param parsedData Données parsées [CONNECT, username]
     * @param socket Socket du client
     */
    void handleConnect(const std::vector<std::string>& parsedData, int socket);
    
    /**
     * @brief Gère la déconnexion d'un client
     * @param parsedData Données parsées [DISCONNECT]
     * @param socket Socket du client
     */
    void handleDisconnect(const std::vector<std::string>& parsedData, int socket);
    
    /**
     * @brief Gère l'envoi d'un message
     * @param parsedData Données parsées [SEND, to, subject, body, timestamp]
     * @param socket Socket de l'expéditeur
     */
    void handleSendMessage(const std::vector<std::string>& parsedData, int socket);
    
    /**
     * @brief Gère un ping
     * @param parsedData Données parsées [PING]
     * @param socket Socket du client
     */
    void handlePing(const std::vector<std::string>& parsedData, int socket);
    
    /**
     * @brief Gère un pong (réponse heartbeat)
     * @param parsedData Données parsées [PONG]
     * @param socket Socket du client
     */
    void handlePong(const std::vector<std::string>& parsedData, int socket);
    
    /**
     * @brief Gère la demande de liste d'utilisateurs
     * @param parsedData Données parsées [LIST_USERS]
     * @param socket Socket du client
     */
    void handleListUsers(const std::vector<std::string>& parsedData, int socket);
    
    /**
     * @brief Gère la demande de logs
     * @param parsedData Données parsées [GET_LOG]
     * @param socket Socket du client
     */
    void handleGetLog(const std::vector<std::string>& parsedData, int socket);    CommandHandler(const CommandHandler&) = delete;
    CommandHandler& operator=(const CommandHandler&) = delete;

private:
    /**
     * @brief Envoie une réponse brute au client
     * @param socket Socket du client
     * @param message Message à envoyer
     */
    void sendResponse(int socket, const std::string& message);
    
    /**
     * @brief Envoie une réponse OK au client
     * @param socket Socket du client
     * @param message Message optionnel (vide par défaut)
     */
    void sendOK(int socket, const std::string& message = "");
    
    /**
     * @brief Envoie une réponse ERROR au client
     * @param socket Socket du client
     * @param error Message d'erreur
     */
    void sendError(int socket, const std::string& error);
    
    Server* server;
};

#endif
