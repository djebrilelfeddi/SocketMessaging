/**
 * @file MessageHandler.cpp
 * @brief Simplified message handler (no display)
 */

#include "Client/MessageHandler.hpp"
#include "Utils/Utils.hpp"
#include "Utils/Constants.hpp"
#include "Utils/Logger.hpp"
#include "Utils/NetworkStream.hpp"
#include "Utils/MessageParser.hpp"

MessageHandler::MessageHandler(int socketFd) : socketFd(socketFd) {
    //LOG_DEBUG("MessageHandler created for socket " + std::to_string(socketFd));
}

bool MessageHandler::sendMessage(const std::string& to, const std::string& subject, const std::string& body) {
    if (!Utils::isValidUsername(to) && to != "all") {
        LOG_ERROR("Invalid recipient: " + to);
        return false;
    }
    
    if (!Utils::isValidSubject(subject)) {
        LOG_ERROR("Invalid subject");
        return false;
    }
    
    return sendCommand(Utils::MessageParser::build("SEND", to, subject, body));
}

bool MessageHandler::sendCommand(const std::string& command) {
    Network::NetworkStream stream(socketFd);
    if (!stream.send(command)) {
        LOG_ERROR("Failed to send command");
        return false;
    }
    return true;
}

void MessageHandler::listen(EventCallback onEvent) {
    Network::NetworkStream stream(socketFd);
    
    while (stream.isConnected()) {
        auto message = stream.receive();
        if (!message) {
            LOG_INFO("Connection closed");
            break;
        }
        
        auto event = parseMessage(*message);
        if (event) {
            // Store received messages
            if (event->type == ServerEvent::MESSAGE) {
                storeMessage(*event);
            }
            // Automatically reply to PING
            else if (event->type == ServerEvent::PING) {
                sendCommand(Utils::MessageParser::build("PONG"));
                LOG_DEBUG("PING received, PONG sent");
                continue;  // No need to notify UI
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
        //LOG_WARNING("Invalid message received");
        return std::nullopt;
    }
    
    ServerEventData event;
    
    if (parsed.command == "MESSAGE" && parsed.argCount() >= 4) {
        event.type = ServerEvent::MESSAGE;
        event.args = {parsed.arg(0), parsed.arg(1), parsed.arg(2), parsed.arg(3)};
    }
    else if (parsed.command == "OK") {
        event.type = ServerEvent::OK;
        event.data = parsed.argCount() > 0 ? parsed.arg(0) : "Operation successful";
    }
    else if (parsed.command == "ERROR") {
        event.type = ServerEvent::ERROR_MSG;
        event.data = parsed.argCount() > 0 ? parsed.arg(0) : "Unknown error";
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
    //LOG_INFO("Message received from " + msg.from + " (index: " + std::to_string(msg.index) + ")");
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
    
    // Search in already read messages
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
