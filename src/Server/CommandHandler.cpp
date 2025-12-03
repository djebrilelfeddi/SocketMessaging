#include "Server/CommandHandler.hpp"
#include "Server/Server.hpp"
#include "Server/Message.hpp"
#include "Utils/Logger.hpp"
#include "Utils/Utils.hpp"
#include "Utils/Constants.hpp"
#include "Utils/NetworkStream.hpp"
#include "Utils/MessageParser.hpp"
#include <fstream>
#include <sstream>
#include <unistd.h>

CommandHandler::CommandHandler(Server* server)
    : server(server) {
}

void CommandHandler::sendResponse(int socket, const std::string& message) {
    Network::NetworkStream stream(socket);
    (void)stream.send(message);
}

void CommandHandler::sendOK(int socket, const std::string& message) {
    std::string msg = message.empty() ? "OK" : message;
    sendResponse(socket, Utils::MessageParser::build("OK", msg));
}

void CommandHandler::sendError(int socket, const std::string& error) {
    sendResponse(socket, Utils::MessageParser::build("ERROR", error));
}

void CommandHandler::handleConnect(const std::vector<std::string>& parsedData, int socket) {
    if (parsedData.size() < 2) {
        LOG_WARNING("Données de connexion invalides");
        return;
    }
    
    std::string username = Utils::sanitize(parsedData[1]);
    
    if (!Utils::isValidUsername(username)) {
        LOG_WARNING("Username invalide: " + username);
        sendError(socket, "Invalid username");
        return;
    }
    
    if (server->isBanned(username)) {
        LOG_WARNING("Tentative de connexion d'un utilisateur banni: " + username);
        sendError(socket, "You are banned from this server");
        close(socket);
        return;
    }
    
    if (server->isUsernameTaken(username)) {
        LOG_WARNING("Username déjà utilisé: " + username);
        sendError(socket, "Username already exists");
        return;
    }
      server->registerClient(username, socket);
    
    LOG_CONNECT("Nouveau client: " + username);
    sendResponse(socket, Utils::MessageParser::build("OK", "Connected as " + username));
}

void CommandHandler::handleDisconnect(const std::vector<std::string>& parsedData, int socket) {
    (void)parsedData;
    std::string username = server->getUsernameBySocket(socket);
    
    if (username.empty()) {
        LOG_WARNING("Tentative de déconnexion d'un client non identifié");
        return;
    }
    
    server->unregisterClient(username);
    
    LOG_DISCONNECT("Client déconnecté: " + username);
    close(socket);
    
    if (Constants::AUTO_STOP_WHEN_NO_CLIENTS && server->getClientCount() == 0) {
        LOG_INFO("Dernier client déconnecté - Arrêt du serveur");
        server->stop();
    }
}

void CommandHandler::handleSendMessage(const std::vector<std::string>& parsedData, int socket) {
    // Erreur 5: Message mal formaté
    if (parsedData.size() < 4) {
        LOG_WARNING("Format de message invalide");
        sendError(socket, "Malformed message: missing fields");
        return;
    }
    
    std::string from = server->getUsernameBySocket(socket);
    if (from.empty()) {
        LOG_WARNING("Tentative d'envoi de message par un client non authentifié");
        sendError(socket, "Not authenticated");
        return;
    }
    
    server->incrementMessagesReceived();
    
    Message msg;
    msg.from = from;
    msg.to = Utils::sanitize(parsedData[1]);
    msg.subject = Utils::sanitize(parsedData[2]);
    msg.body = Utils::sanitize(parsedData[3]);
    msg.timestamp = std::chrono::system_clock::now();
    
    if (!Utils::isValidSubject(msg.subject)) {
        LOG_WARNING("Sujet invalide de " + from + " (max " + std::to_string(Constants::MAX_SUBJECT_LENGTH) + " caractères)");
        sendError(socket, "Subject too long (max " + std::to_string(Constants::MAX_SUBJECT_LENGTH) + " chars)");
        return;
    }
    
    if (!Utils::isValidBody(msg.body)) {
        LOG_WARNING("Corps du message invalide de " + from);
        sendError(socket, "Body is empty");
        return;
    }
    
    if (msg.to == "all") {
        LOG_INFO("Broadcast de " + from);
        auto clients = server->getAllClients();
        
        for (const auto& [username, userSocket] : clients) {
            if (username != from) {
                Message broadcastMsg = msg;
                broadcastMsg.to = username;
                auto dispatcher = server->getDispatcher();
                if (dispatcher) {
                    dispatcher->queueMessage(broadcastMsg);
                }
            }
        }
        
        sendOK(socket, "Broadcast envoyé");
        return;
    }
    
    // Erreur 2: Utilisateur destinataire n'existe pas
    int recipientSocket = server->getUserSocket(msg.to);
    if (recipientSocket <= 0) {
        LOG_WARNING("Destinataire inexistant: " + msg.to + " (de " + from + ")");
        sendError(socket, "User '" + msg.to + "' does not exist or is offline");
        return;
    }
    
    auto dispatcher = server->getDispatcher();
    // Erreur 3: L'émission n'a pas pu être exécutée
    if (dispatcher && dispatcher->queueMessage(msg)) {
        LOG_DEBUG("Message de " + from + " ajouté à la queue");
        sendOK(socket, "Message envoyé");
    } else {
        LOG_ERROR("Échec de l'ajout du message à la queue");
        sendError(socket, "Failed to send message: queue full or dispatcher error");
    }
}

void CommandHandler::handlePing(const std::vector<std::string>& parsedData, int socket) {
    (void)parsedData;
    sendResponse(socket, "PONG\n");
    LOG_DEBUG("PING reçu, PONG envoyé");
}

void CommandHandler::handlePong(const std::vector<std::string>& parsedData, int socket) {
    (void)parsedData;
    std::string username = server->getUsernameBySocket(socket);
    
    if (username.empty()) {
        return;
    }
    
    server->updateClientPong(username);
    LOG_DEBUG("PONG reçu de " + username);
}

void CommandHandler::handleListUsers(const std::vector<std::string>& parsedData, int socket) {
    (void)parsedData;
    auto clients = server->getAllClients();
    
    std::string userList;
    for (const auto& [username, userSocket] : clients) {
        userList += username + ",";
    }
    
    if (!userList.empty() && userList.back() == ',') {
        userList.pop_back();
    }
    
    sendResponse(socket, Utils::MessageParser::build("USERS", userList));
    LOG_DEBUG("Liste des utilisateurs envoyée");
}

void CommandHandler::handleGetLog(const std::vector<std::string>& parsedData, int socket) {
    (void)parsedData;
    
    std::ifstream logFile(Constants::DEFAULT_SERVER_LOG);
    if (!logFile.is_open()) {
        LOG_WARNING("Impossible d'ouvrir le fichier de log: " + Constants::DEFAULT_SERVER_LOG);
        sendError(socket, "Log file not available");
        return;
    }
    
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(logFile, line)) {
        lines.push_back(line);
    }
    logFile.close();
    
    if (lines.empty()) {
        sendResponse(socket, Utils::MessageParser::build("LOG", "Log file is empty"));
        LOG_DEBUG("Log vide envoyé");
        return;
    }
    
    size_t start = lines.size() > 50 ? lines.size() - 50 : 0;
    std::string logContent;
    for (size_t i = start; i < lines.size(); ++i) {
        logContent += lines[i] + "\n";
    }
    
    sendResponse(socket, Utils::MessageParser::build("LOG", logContent));
    LOG_DEBUG("Log envoyé (" + std::to_string(lines.size() - start) + " lignes)");
}
