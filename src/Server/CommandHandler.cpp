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
        LOG_WARNING("Invalid connection data");
        return;
    }
    
    std::string username = Utils::sanitize(parsedData[1]);
    
    if (!Utils::isValidUsername(username)) {
        LOG_WARNING("Invalid username: " + username);
        sendError(socket, "Invalid username");
        return;
    }
    
    if (server->isBanned(username)) {
        LOG_WARNING("Banned user connection attempt: " + username);
        sendError(socket, "You are banned from this server");
        close(socket);
        return;
    }
    
    if (server->isUsernameTaken(username)) {
        LOG_WARNING("Username already taken: " + username);
        sendError(socket, "Username already exists");
        return;
    }
      server->registerClient(username, socket);
    
    LOG_CONNECT("New client: " + username);
    sendResponse(socket, Utils::MessageParser::build("OK", "Connected as " + username));
}

void CommandHandler::handleDisconnect(const std::vector<std::string>& parsedData, int socket) {
    (void)parsedData;
    std::string username = server->getUsernameBySocket(socket);
    
    if (username.empty()) {
        LOG_WARNING("Disconnect attempt from unidentified client");
        return;
    }
    
    server->unregisterClient(username);
    
    LOG_DISCONNECT("Client disconnected: " + username);
    close(socket);
    
    if (Constants::AUTO_STOP_WHEN_NO_CLIENTS && server->getClientCount() == 0) {
        LOG_INFO("Last client disconnected - Stopping server");
        server->stop();
    }
}

void CommandHandler::handleSendMessage(const std::vector<std::string>& parsedData, int socket) {
    // Error 5: Malformed message
    if (parsedData.size() < 4) {
        LOG_WARNING("Invalid message format");
        sendError(socket, "Malformed message: missing fields");
        return;
    }
    
    std::string from = server->getUsernameBySocket(socket);
    if (from.empty()) {
        LOG_WARNING("Message send attempt by unauthenticated client");
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
        LOG_WARNING("Invalid subject from " + from + " (max " + std::to_string(Constants::MAX_SUBJECT_LENGTH) + " characters)");
        sendError(socket, "Subject too long (max " + std::to_string(Constants::MAX_SUBJECT_LENGTH) + " chars)");
        return;
    }
    
    if (!Utils::isValidBody(msg.body)) {
        LOG_WARNING("Invalid message body from " + from);
        sendError(socket, "Body is empty");
        return;
    }
    
    if (msg.to == "all") {
        LOG_INFO("Broadcast from " + from);
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
        
        sendOK(socket, "Broadcast sent");
        return;
    }
    
    // Error 2: Recipient user does not exist
    int recipientSocket = server->getUserSocket(msg.to);
    if (recipientSocket <= 0) {
        LOG_WARNING("Non-existent recipient: " + msg.to + " (from " + from + ")");
        sendError(socket, "User '" + msg.to + "' does not exist or is offline");
        return;
    }
    
    auto dispatcher = server->getDispatcher();
    // Error 3: Sending could not be executed
    if (dispatcher && dispatcher->queueMessage(msg)) {
        LOG_DEBUG("Message from " + from + " added to queue");
        sendOK(socket, "Message sent");
    } else {
        LOG_ERROR("Failed to add message to queue");
        sendError(socket, "Failed to send message: queue full or dispatcher error");
    }
}

void CommandHandler::handlePing(const std::vector<std::string>& parsedData, int socket) {
    (void)parsedData;
    sendResponse(socket, "PONG\n");
    LOG_DEBUG("PING received, PONG sent");
}

void CommandHandler::handlePong(const std::vector<std::string>& parsedData, int socket) {
    (void)parsedData;
    std::string username = server->getUsernameBySocket(socket);
    
    if (username.empty()) {
        return;
    }
    
    server->updateClientPong(username);
    LOG_DEBUG("PONG received from " + username);
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
    LOG_DEBUG("User list sent");
}

void CommandHandler::handleGetLog(const std::vector<std::string>& parsedData, int socket) {
    (void)parsedData;
    
    std::ifstream logFile(Constants::DEFAULT_SERVER_LOG);
    if (!logFile.is_open()) {
        LOG_WARNING("Cannot open log file: " + Constants::DEFAULT_SERVER_LOG);
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
        LOG_DEBUG("Empty log sent");
        return;
    }
    
    size_t start = lines.size() > 50 ? lines.size() - 50 : 0;
    std::string logContent;
    for (size_t i = start; i < lines.size(); ++i) {
        logContent += lines[i] + "\n";
    }
    
    sendResponse(socket, Utils::MessageParser::build("LOG", logContent));
    LOG_DEBUG("Log sent (" + std::to_string(lines.size() - start) + " lines)");
}
