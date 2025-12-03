/**
 * @file ClientUI.cpp
 * @brief Interface utilisateur du client (tout l'affichage est ici)
 */

#include "Client/ClientUI.hpp"
#include "Utils/Logger.hpp"
#include "Utils/Utils.hpp"
#include "Utils/MessageParser.hpp"
#include "Utils/Colors.hpp"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <thread>

ClientUI::ClientUI(Client& client, const std::string& serverIp, int serverPort)
    : client(client), serverIp(serverIp), serverPort(serverPort) {
    
    commands["1"] = [this]() { cmdSendMessage(false); };
    commands["2"] = [this]() { cmdListUnread(); };
    commands["3"] = [this]() { cmdReadMessage(); };
    commands["4"] = [this]() { cmdListUsers(); };
    commands["5"] = [this]() { cmdSendMessage(true); };
    commands["6"] = [this]() { cmdGetLog(); };
}

// === Affichage ===

void ClientUI::clearScreen() {
    std::cout << "\033[2J\033[1;1H" << std::flush;
}

void ClientUI::printHeader() {
    clearScreen();
    std::cout << Color::BRIGHT_CYAN << "========================================\n";
    std::cout << "   CLIENT DE MESSAGERIE\n";
    std::cout << "========================================" << Color::RESET << "\n";
    std::cout << "Serveur: " << serverIp << ":" << serverPort << "\n\n";
}

void ClientUI::print(const std::string& msg) {
    std::cout << msg;
}

void ClientUI::printError(const std::string& msg) {
    std::cout << Color::BRIGHT_RED << "[ERREUR] " << Color::RESET << msg << "\n";
}

void ClientUI::printSuccess(const std::string& msg) {
    std::cout << Color::BRIGHT_GREEN << "[OK] " << Color::RESET << msg << "\n";
}

void ClientUI::printMenu() {
    std::cout << "\n" << Color::BRIGHT_CYAN << client.getCurrentUsername() << Color::RESET 
              << " - Connecté à " << Color::CYAN << serverIp << ":" << serverPort << Color::RESET << "\n";
    std::cout << "\n" << Color::BRIGHT_MAGENTA << "=== MENU ===" << Color::RESET << "\n";
    std::cout << "1. Envoyer un message\n";
    std::cout << "2. Messages non lus\n";
    std::cout << "3. Lire un message\n";
    std::cout << "4. Utilisateurs en ligne\n";
    std::cout << "5. Broadcast\n";
    std::cout << "6. Logs serveur\n";
    std::cout << Color::BRIGHT_RED << "7. Quitter" << Color::RESET << "\n";
    std::cout << Color::YELLOW << "$ " << Color::RESET;
}

void ClientUI::promptAndWait() {
    std::cout << "\n" << Color::GRAY << "Entrée pour continuer..." << Color::RESET;
    std::cin.get();
    clearScreen();
}

void ClientUI::printUserList(const std::string& userListData) {
    auto users = Utils::split(userListData, ",");
    std::cout << Color::BRIGHT_GREEN << "=== Utilisateurs (" << users.size() << ") ===" << Color::RESET << "\n";
    for (const auto& user : users) {
        if (user == client.getCurrentUsername()) {
            std::cout << "  " << Color::BRIGHT_YELLOW << "* " << user << " (Vous)" << Color::RESET << "\n";
        } else {
            std::cout << "  " << user << "\n";
        }
    }
}

// === Connexion ===

bool ClientUI::promptAndConnect() {
    printHeader();
    
    std::string username;
    std::cout << Color::YELLOW << "Nom d'utilisateur: " << Color::RESET;
    std::getline(std::cin, username);
    
    if (!Utils::isValidUsername(username)) {
        printError("Nom invalide (alphanumérique et underscore, max 16 caractères)");
        return false;
    }
    
    std::string errorMsg;
    if (!client.connect(username, errorMsg)) {
        printError(errorMsg.empty() ? "Connexion échouée" : errorMsg);
        return false;
    }
    
    printSuccess("Connecté!");
    return true;
}

// === Commandes ===

void ClientUI::cmdSendMessage(bool broadcast) {
    auto* handler = client.getMessageHandler();
    if (!handler) return;
    
    std::string to, subject, body;
    
    if (broadcast) {
        to = "all";
        std::cout << Color::BRIGHT_MAGENTA << "Envoi à tous" << Color::RESET << "\n";
    } else {
        std::cout << "Destinataire: ";
        std::getline(std::cin, to);
    }
    
    std::cout << "Sujet: ";
    std::getline(std::cin, subject);
    
    std::cout << "Message: ";
    std::getline(std::cin, body);
    
    if (!handler->sendMessage(to, subject, body)) {
        printError("Échec de l'envoi");
    }
}

void ClientUI::cmdListUnread() {
    auto* handler = client.getMessageHandler();
    if (!handler) return;
    
    auto messages = handler->getUnreadMessages();
    
    if (messages.empty()) {
        std::cout << Color::YELLOW << "Aucun message non lu." << Color::RESET << "\n";
        return;
    }
    
    std::cout << "\n" << Color::BRIGHT_MAGENTA << "=== Messages non lus (" << messages.size() << ") ===" << Color::RESET << "\n";
    for (const auto& msg : messages) {
        std::cout << Color::CYAN << "[" << msg.index << "]" << Color::RESET << " De: " << std::setw(12) << msg.from << " | " << msg.subject << "\n";
    }
}

void ClientUI::cmdReadMessage() {
    auto* handler = client.getMessageHandler();
    if (!handler) return;
    
    std::cout << "Index: ";
    int index;
    std::cin >> index;
    std::cin.ignore();
    
    ReceivedMessage msg;
    if (!handler->readMessageByIndex(index, msg)) {
        printError("Message introuvable");
        return;
    }
    
    std::cout << "\n" << Color::BRIGHT_CYAN << "=== Message #" << index << " ===" << Color::RESET << "\n";
    std::cout << "De: " << msg.from << "\n";
    std::cout << "Sujet: " << msg.subject << "\n";
    std::cout << "Date: " << Utils::formatTimestamp(msg.timestamp) << "\n";
    std::cout << "---\n" << msg.body << "\n";
    
    std::cout << "\n[r] Répondre | [Entrée] Retour: ";
    std::string action;
    std::getline(std::cin, action);
    
    if (action == "r" || action == "R") {
        std::cout << "Réponse: ";
        std::string reply;
        std::getline(std::cin, reply);
        if (!reply.empty()) {
            handler->replyToMessage(index, reply);
        }
    }
}

void ClientUI::cmdListUsers() {
    auto* handler = client.getMessageHandler();
    if (handler) {
        handler->sendCommand(Utils::MessageParser::build("LIST_USERS"));
        // Attendre la réponse du serveur
        waitForResponse(ServerEvent::USERS);
    }
}

void ClientUI::cmdGetLog() {
    auto* handler = client.getMessageHandler();
    if (handler) {
        handler->sendCommand(Utils::MessageParser::build("GET_LOG"));
        // Attendre la réponse du serveur
        waitForResponse(ServerEvent::LOG);
    }
}

// === Gestion des événements ===

void ClientUI::waitForResponse(ServerEvent expectedType, int timeoutMs) {
    // Temporairement accepter les événements directs pour affichage
    inCommand.store(false);
    
    auto start = std::chrono::steady_clock::now();
    while (true) {
        // Vérifier le timeout
        auto elapsed = std::chrono::steady_clock::now() - start;
        if (std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() > timeoutMs) {
            break;
        }
        
        // Vérifier si on a reçu la réponse attendue
        {
            std::lock_guard<std::mutex> lock(eventQueueMutex);
            if (lastReceivedEvent == expectedType) {
                lastReceivedEvent = ServerEvent::NONE;
                break;
            }
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    
    // Remettre en mode commande
    inCommand.store(true);
}

void ClientUI::onServerEvent(const ServerEventData& event) {
    // Toujours mettre en file d'attente si on attend une entrée utilisateur
    if (inCommand.load()) {
        std::lock_guard<std::mutex> lock(eventQueueMutex);
        if (event.type != ServerEvent::OK) pendingEvents.push(event);
        return;
    }
    
    // Affichage immédiat seulement quand on est au menu (après affichage du menu)
    displayEvent(event);
    
    // Enregistrer le type pour waitForResponse
    {
        std::lock_guard<std::mutex> lock(eventQueueMutex);
        lastReceivedEvent = event.type;
    }
}

void ClientUI::displayEvent(const ServerEventData& event) {
    switch (event.type) {
        case ServerEvent::MESSAGE:
            std::cout << "\n" << Color::BRIGHT_YELLOW << "Nouveau Message: " 
                      << event.args[0] << " - " << event.args[1] << Color::RESET << "\n$ " << std::flush;
            break;
        case ServerEvent::OK:
            std::cout << "\n" << Color::BRIGHT_GREEN << "[OK] " << Color::RESET << event.data << "\n$ " << std::flush;
            break;
        case ServerEvent::ERROR_MSG:
            std::cout << "\n" << Color::BRIGHT_RED << "[ERREUR] " << Color::RESET << event.data << "\n$ " << std::flush;
            break;
        case ServerEvent::USERS:
            std::cout << "\n";
            printUserList(event.data);
            std::cout << "$ " << std::flush;
            break;
        case ServerEvent::LOG:
            std::cout << "\n" << Color::BRIGHT_CYAN << "=== LOG ===" << Color::RESET << "\n";
            std::cout << event.data << "\n$ " << std::flush;
            break;
        default:
            break;
    }
}

void ClientUI::displayPendingEvents() {
    std::lock_guard<std::mutex> lock(eventQueueMutex);
    
    if (pendingEvents.empty()) return;
    
    std::cout << "\n" << Color::GRAY << "--- Pendant que vous étiez occupé ---" << Color::RESET << "\n";
    while (!pendingEvents.empty()) {
        auto& event = pendingEvents.front();
        switch (event.type) {
            case ServerEvent::MESSAGE:
                std::cout << Color::BRIGHT_GREEN << "Nouveau message de " 
                          << event.args[0] << " - " << event.args[1] << Color::RESET << "\n";
                break;
            case ServerEvent::ERROR_MSG:
                std::cout << Color::BRIGHT_RED << "[ERREUR] " << Color::RESET << event.data << "\n";
                break;
            case ServerEvent::USERS:
                printUserList(event.data);
                break;
            case ServerEvent::LOG:
                std::cout << Color::BRIGHT_CYAN << "=== LOG ===" << Color::RESET << "\n";
                std::cout << event.data << "\n";
                break;
            default:
                break;
        }
        pendingEvents.pop();
    }
}

// === Boucle principale ===

bool ClientUI::run() {
    if (!promptAndConnect()) {
        return false;
    }
    
    clearScreen();
    
    // Démarrer l'écoute avec callback vers l'UI
    client.startListening([this](const ServerEventData& event) {
        onServerEvent(event);
    });
    
    bool running = true;
    while (running) {
        // Afficher le menu puis libérer pour recevoir les événements
        printMenu();
        inCommand.store(false);
        
        std::string choice;
        std::getline(std::cin, choice);
        
        // Bloquer les événements pendant le traitement
        inCommand.store(true);
        clearScreen();
        
        if (choice == "7") {
            std::cout << Color::YELLOW << "Déconnexion..." << Color::RESET << "\n";
            running = false;
        } else {
            auto it = commands.find(choice);
            if (it != commands.end()) {
                it->second();
                promptAndWait();
                displayPendingEvents();
            } else {
                printError("Choix invalide");
                promptAndWait();
            }
        }
    }
    
    client.disconnect();
    return true;
}
