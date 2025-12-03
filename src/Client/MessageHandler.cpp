/**
 * @file MessageHandler.cpp
 * @brief Gestionnaire de messages simplifié (sans affichage)
 */

#include "Client/MessageHandler.hpp"
#include "Utils/Utils.hpp"
#include "Utils/Constants.hpp"
#include "Utils/Logger.hpp"
#include "Utils/NetworkStream.hpp"
#include "Utils/MessageParser.hpp"

MessageHandler::MessageHandler(int socketFd) : socketFd(socketFd) {
    //LOG_DEBUG("MessageHandler créé pour socket " + std::to_string(socketFd));
}

bool MessageHandler::sendMessage(const std::string& to, const std::string& subject, const std::string& body) {
    if (!Utils::isValidUsername(to) && to != "all") {
        LOG_ERROR("Destinataire invalide: " + to);
        return false;
    }
    
    if (!Utils::isValidSubject(subject)) {
        LOG_ERROR("Sujet invalide");
        return false;
    }
    
    return sendCommand(Utils::MessageParser::build("SEND", to, subject, body));
}

bool MessageHandler::sendCommand(const std::string& command) {
    Network::NetworkStream stream(socketFd);
    if (!stream.send(command)) {
        LOG_ERROR("Échec de l'envoi de la commande");
        return false;
    }
    return true;
}

void MessageHandler::listen(EventCallback onEvent) {
    Network::NetworkStream stream(socketFd);
    
    while (stream.isConnected()) {
        auto message = stream.receive();
        if (!message) {
            LOG_INFO("Connexion fermée");
            break;
        }
        
        auto event = parseMessage(*message);
        if (event) {
            // Stocker les messages reçus
            if (event->type == ServerEvent::MESSAGE) {
                storeMessage(*event);
            }
            // Répondre automatiquement au PING
            else if (event->type == ServerEvent::PING) {
                sendCommand(Utils::MessageParser::build("PONG"));
                LOG_DEBUG("PING reçu, PONG envoyé");
                continue;  // Pas besoin de notifier l'UI
            }
            
            if (onEvent) {
                onEvent(*event);
            }
        }
    }
}

std::optional<ServerEventData> MessageHandler::parseMessage(const std::string& raw) {
    auto parsed = Utils::MessageParser::parse(raw);
    if (!parsed.isValid) {
        //LOG_WARNING("Message invalide reçu");
        return std::nullopt;
    }
    
    ServerEventData event;
    
    if (parsed.command == "MESSAGE" && parsed.argCount() >= 4) {
        event.type = ServerEvent::MESSAGE;
        event.args = {parsed.arg(0), parsed.arg(1), parsed.arg(2), parsed.arg(3)};
    }
    else if (parsed.command == "OK") {
        event.type = ServerEvent::OK;
        event.data = parsed.argCount() > 0 ? parsed.arg(0) : "Opération réussie";
    }
    else if (parsed.command == "ERROR") {
        event.type = ServerEvent::ERROR_MSG;
        event.data = parsed.argCount() > 0 ? parsed.arg(0) : "Erreur inconnue";
    }
    else if (parsed.command == "USERS" && parsed.argCount() >= 1) {
        event.type = ServerEvent::USERS;
        event.data = parsed.arg(0);
    }
    else if (parsed.command == "LOG" && parsed.argCount() >= 1) {
        event.type = ServerEvent::LOG;
        event.data = parsed.arg(0);
    }
    else if (parsed.command == "PING") {
        event.type = ServerEvent::PING;
    }
    else {
        return std::nullopt;
    }
    
    return event;
}

void MessageHandler::storeMessage(const ServerEventData& data) {
    std::lock_guard<std::mutex> lock(messagesMutex);
    
    ReceivedMessage msg;
    msg.from = data.args[0];
    msg.subject = data.args[1];
    msg.body = data.args[2];
    msg.timestamp = Utils::unixStringToTimestamp(data.args[3]);
    msg.receivedAt = std::chrono::system_clock::now();
    msg.isRead = false;
    msg.index = messageCounter++;
    
    unreadMessages.push_back(msg);
    //LOG_INFO("Message reçu de " + msg.from + " (index: " + std::to_string(msg.index) + ")");
}

std::vector<ReceivedMessage> MessageHandler::getUnreadMessages() const {
    std::lock_guard<std::mutex> lock(messagesMutex);
    return unreadMessages;
}

int MessageHandler::getUnreadCount() const {
    std::lock_guard<std::mutex> lock(messagesMutex);
    return static_cast<int>(unreadMessages.size());
}

bool MessageHandler::readMessageByIndex(int index, ReceivedMessage& out) {
    std::lock_guard<std::mutex> lock(messagesMutex);
    
    for (auto it = unreadMessages.begin(); it != unreadMessages.end(); ++it) {
        if (it->index == index) {
            out = *it;
            it->isRead = true;
            readMessages.push_back(*it);
            unreadMessages.erase(it);
            return true;
        }
    }
    
    // Chercher dans les messages déjà lus
    for (auto& msg : readMessages) {
        if (msg.index == index) {
            out = msg;
            return true;
        }
    }
    
    return false;
}

bool MessageHandler::replyToMessage(int originalIndex, const std::string& body) {
    ReceivedMessage original;
    {
        std::lock_guard<std::mutex> lock(messagesMutex);
        bool found = false;
        for (const auto& msg : unreadMessages) {
            if (msg.index == originalIndex) { original = msg; found = true; break; }
        }
        if (!found) {
            for (const auto& msg : readMessages) {
                if (msg.index == originalIndex) { original = msg; found = true; break; }
            }
        }
        if (!found) return false;
    }
    
    std::string replySubject = (original.subject.substr(0, 4) == "Re: ") 
        ? original.subject 
        : "Re: " + original.subject;
    
    return sendMessage(original.from, replySubject, body);
}
