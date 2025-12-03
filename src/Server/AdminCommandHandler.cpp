#include "Server/AdminCommandHandler.hpp"
#include "Server/Server.hpp"
#include "Utils/Logger.hpp"
#include "Utils/Utils.hpp"
#include "Utils/NetworkStream.hpp"
#include "Utils/MessageParser.hpp"
#include "Utils/Constants.hpp"
#include "Utils/RuntimeConfig.hpp"
#include <iostream>
#include <iomanip>
#include <unistd.h>

AdminCommandHandler::AdminCommandHandler(Server* server)
    : server(server) {
    initializeCommands();
}

void AdminCommandHandler::initializeCommands() {
    auto reg = [this](const std::string& name, void (AdminCommandHandler::*method)(const std::vector<std::string>&),
                      const std::string& usage, const std::string& desc, int minArgs) {
        commands[name] = { [this, method](const auto& args) { (this->*method)(args); }, usage, desc, minArgs };
    };
    
    reg("help",      &AdminCommandHandler::cmdHelp,      "/help",                  "Afficher cette aide",      0);
    reg("broadcast", &AdminCommandHandler::cmdBroadcast, "/broadcast <message>",   "Envoyer à tous",           1);
    reg("send",      &AdminCommandHandler::cmdSend,      "/send <user> <message>", "Envoyer à un utilisateur", 2);
    reg("list",      &AdminCommandHandler::cmdList,      "/list",                  "Lister les clients",       0);
    reg("kick",      &AdminCommandHandler::cmdKick,      "/kick <user>",           "Déconnecter un client",    1);
    reg("ban",       &AdminCommandHandler::cmdBan,       "/ban <user>",            "Bannir un client",         1);
    reg("unban",     &AdminCommandHandler::cmdUnban,     "/unban <user>",          "Débannir un client",       1);
    reg("stats",     &AdminCommandHandler::cmdStats,     "/stats",                 "Afficher les stats",       0);
    reg("set",       &AdminCommandHandler::cmdSet,       "/set <nom> <valeur>",    "Modifier une config",      2);
    reg("config",    &AdminCommandHandler::cmdConfig,    "/config",                "Lister les configs",       0);
    reg("reset",     &AdminCommandHandler::cmdReset,     "/reset",                 "Réinitialiser configs",    0);
    reg("stop",      &AdminCommandHandler::cmdStop,      "/stop",                  "Arrêter le serveur",       0);
}

void AdminCommandHandler::commandLoop() {
    while (running && server->getStatus() == SERVER_STATUS::RUNNING) {
        std::string line;
        if (!std::getline(std::cin, line)) {
            break;
        }
        
        if (line.empty()) continue;
        
        if (line[0] != '/') {
            std::cout << "Les commandes doivent commencer par '/'. Tapez /help\n";
            continue;
        }
        
        auto parts = Utils::split(line.substr(1), " ");
        if (parts.empty()) continue;
        
        std::string cmdName = parts[0];
        auto it = commands.find(cmdName);
        
        if (it == commands.end()) {
            std::cout << "Commande inconnue: /" << cmdName << "\n";
            continue;
        }
        
        const auto& cmd = it->second;
        if (static_cast<int>(parts.size()) - 1 < cmd.minArgs) {
            std::cout << "Usage: " << cmd.usage << "\n";
            continue;
        }
        
        cmd.handler(parts);
        std::cout << "admin> ";
    }
}

// === Commandes ===

void AdminCommandHandler::cmdHelp(const std::vector<std::string>&) {
    std::cout << "Commandes disponibles:\n";
    for (const auto& [name, cmd] : commands) {
        std::cout << "  " << std::left << std::setw(24) << cmd.usage << " - " << cmd.description << "\n";
    }
}

void AdminCommandHandler::cmdBroadcast(const std::vector<std::string>& args) {
    std::string message;
    for (size_t i = 1; i < args.size(); ++i) {
        if (i > 1) message += " ";
        message += args[i];
    }
    
    auto clients = server->getAllClients();
    if (clients.empty()) {
        std::cout << "[Admin] Aucun client connecté\n";
        return;
    }
    
    int sent = 0;
    for (const auto& [username, socket] : clients) {
        Network::NetworkStream stream(socket);
        if (stream.send(Utils::MessageParser::build("MESSAGE", "SERVEUR", "Annonce", message, "0"))) {
            sent++;
        }
    }
    
    std::cout << "[Admin] Broadcast envoyé à " << sent << " client(s)\n";
    LOG_INFO("Admin broadcast: " + message);
}

void AdminCommandHandler::cmdSend(const std::vector<std::string>& args) {
    std::string username = args[1];
    
    // Reconstituer le message
    std::string message;
    for (size_t i = 2; i < args.size(); ++i) {
        if (i > 2) message += " ";
        message += args[i];
    }
    
    int socket = server->getUserSocket(username);
    if (socket <= 0) {
        std::cout << "[Admin] Utilisateur '" << username << "' non trouvé\n";
        return;
    }
    
    Network::NetworkStream stream(socket);
    if (stream.send(Utils::MessageParser::build("MESSAGE", "SERVEUR", "Message Privé", message, "0"))) {
        std::cout << "[Admin] Message envoyé à " << username << "\n";
        LOG_INFO("Admin message to " + username + ": " + message);
    } else {
        std::cout << "[Admin] Échec de l'envoi\n";
    }
}

void AdminCommandHandler::cmdList(const std::vector<std::string>&) {
    auto clients = server->getAllClients();
    
    if (clients.empty()) {
        std::cout << "[Admin] Aucun client connecté\n";
        return;
    }
    
    std::cout << "\n=== Clients connectés (" << clients.size() << ") ===\n";
    for (const auto& [username, socket] : clients) {
        std::cout << "  - " << username << " (socket: " << socket << ")\n";
    }
    std::cout << "================================\n\n";
}

void AdminCommandHandler::cmdKick(const std::vector<std::string>& args) {
    const std::string& username = args[1];
    if (disconnectUser(username, "Vous avez été déconnecté par l'admin")) {
        std::cout << "[Admin] Utilisateur '" << username << "' déconnecté\n";
        LOG_INFO("Admin kicked user: " + username);
    }
}

void AdminCommandHandler::cmdBan(const std::vector<std::string>& args) {
    const std::string& username = args[1];
    if (disconnectUser(username, "Vous avez été banni par l'admin")) {
        server->banlistAdd(username);
        std::cout << "[Admin] Utilisateur '" << username << "' banni et déconnecté\n";
        LOG_INFO("Admin banned user: " + username);
    }
}

void AdminCommandHandler::cmdUnban(const std::vector<std::string>& args) {
    const std::string& username = args[1];
    if (server->banlistRemove(username)) {
        std::cout << "[Admin] Utilisateur '" << username << "' débanni\n";
        LOG_INFO("Admin unbanned user: " + username);
    } else {
        std::cout << "[Admin] Utilisateur '" << username << "' n'est pas dans la banlist\n";
    }
}

void AdminCommandHandler::cmdStats(const std::vector<std::string>&) {
    auto clients = server->getAllClients();
    auto startTime = server->getStartTime();
    auto totalMessagesReceived = server->getTotalMessagesReceived();
    auto totalMessagesSent = server->getTotalMessagesSent();
    auto cfg = server->getConfig();
    
    auto now = std::chrono::steady_clock::now();
    auto uptime = std::chrono::duration_cast<std::chrono::seconds>(now - startTime);
    int hours = static_cast<int>(uptime.count() / 3600);
    int minutes = static_cast<int>((uptime.count() % 3600) / 60);
    int seconds = static_cast<int>(uptime.count() % 60);
    
    double avgMessagesPerMinute = 0.0;
    if (uptime.count() > 0) {
        avgMessagesPerMinute = static_cast<double>(totalMessagesReceived + totalMessagesSent) / (uptime.count() / 60.0);
    }
    
    std::cout << "\n========== STATISTIQUES ==========\n";
    std::cout << "Port:              " << cfg.port << "\n";
    std::cout << "Uptime:            " 
              << std::setw(2) << std::setfill('0') << hours << ":"
              << std::setw(2) << std::setfill('0') << minutes << ":"
              << std::setw(2) << std::setfill('0') << seconds << "\n";
    std::cout << std::setfill(' ');
    std::cout << "-----------------------------------\n";
    std::cout << "Clients:           " << clients.size() << "\n";
    std::cout << "Messages reçus:    " << totalMessagesReceived << "\n";
    std::cout << "Messages envoyés:  " << totalMessagesSent << "\n";
    std::cout << "Messages/min:      " << std::fixed << std::setprecision(2) << avgMessagesPerMinute << "\n";
    std::cout << "===================================\n";
    
    if (!clients.empty()) {
        std::cout << "\nClients en ligne:\n";
        int i = 1;
        for (const auto& [username, socket] : clients) {
            std::cout << "  " << i++ << ". " << username << "\n";
        }
    }
    std::cout << "\n";
}

void AdminCommandHandler::cmdSet(const std::vector<std::string>& args) {
    const std::string& key = args[1];
    const std::string& value = args[2];
    
    if (RuntimeConfig::getInstance().set(key, value)) {
        std::cout << "[OK] " << key << " = " << value << "\n";
    } else {
        std::cout << "[ÉCHEC] Impossible de modifier " << key << "\n";
    }
}

void AdminCommandHandler::cmdConfig(const std::vector<std::string>&) {
    auto configs = RuntimeConfig::getInstance().listAll();
    
    std::cout << "\n========== CONFIGURATIONS ==========\n";
    for (const auto& [key, value] : configs) {
        std::cout << "  " << std::left << std::setw(28) << key << " = " << value << "\n";
    }
    std::cout << "=====================================\n\n";
}

void AdminCommandHandler::cmdReset(const std::vector<std::string>&) {
    RuntimeConfig::getInstance().reset();
    std::cout << "[OK] Configurations réinitialisées\n";
}

void AdminCommandHandler::cmdStop(const std::vector<std::string>&) {
    std::cout << "Arrêt du serveur...\n";
    running = false;
    server->stop();
}

// === Helpers ===

bool AdminCommandHandler::disconnectUser(const std::string& username, const std::string& reason) {
    int socket = server->getUserSocket(username);
    
    if (socket <= 0) {
        std::cout << "[Admin] Utilisateur '" << username << "' non trouvé\n";
        return false;
    }
    
    Network::NetworkStream stream(socket);
    (void)stream.send(Utils::MessageParser::build("ERROR", reason));
    
    server->unregisterClient(username);
    close(socket);
    return true;
}
