#include "Server/Server.hpp"
#include "Server/AdminCommandHandler.hpp"
#include "Server/CommandHandler.hpp"
#include "Utils/Logger.hpp"
#include "Utils/Utils.hpp"
#include "Utils/Constants.hpp"
#include "Utils/NetworkStream.hpp"
#include "Utils/MessageParser.hpp"
#include <cstdlib>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>

Server::Server() {
    adminHandler = std::make_unique<AdminCommandHandler>(this);
    commandHandler = std::make_unique<::CommandHandler>(this);
    LOG_INFO("Server created with default configuration");
}

Server::Server(const ServerConfig& configuration) 
    : config(configuration) {
    adminHandler = std::make_unique<AdminCommandHandler>(this);
    commandHandler = std::make_unique<::CommandHandler>(this);
    LOG_INFO("Server created with custom configuration");
}

Server::~Server() {
    stop();
}

int Server::start(int PORT, int MAX_CONNECTIONS) {
    if (status != SERVER_STATUS::OFF) {
        LOG_ERROR("Server is already started");
        return -1;
    }
    
    status = SERVER_STATUS::STARTING;
    LOG_INFO("Starting server on port " + std::to_string(PORT));
    
    initializeConfig(PORT);
    config.max_connections = (MAX_CONNECTIONS > 0) ? MAX_CONNECTIONS : 100;
    
    config.socket = socket(AF_INET, SOCK_STREAM, 0);
    if (config.socket < 0) {
        LOG_ERROR("Failed to create socket");
        status = SERVER_STATUS::OFF;
        return -1;
    }
    
    int opt = 1;
    if (setsockopt(config.socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        LOG_WARNING("Failed to configure SO_REUSEADDR");
    }
    
    if (bind(config.socket, (struct sockaddr*)&config.address, sizeof(config.address)) < 0) {
        LOG_ERROR("Failed to bind on port " + std::to_string(PORT));
        close(config.socket);
        status = SERVER_STATUS::OFF;
        return -1;
    }
    
    if (listen(config.socket, config.max_connections) < 0) {
        LOG_ERROR("Failed to listen");
        close(config.socket);
        status = SERVER_STATUS::OFF;
        return -1;
    }
    dispatcher = std::make_unique<Dispatcher>(this);
    threadPool = std::make_unique<ThreadPool>(Constants::THREAD_POOL_SIZE);
    
    initializeCommands();
    loadBanlist();
    
    startTime = std::chrono::steady_clock::now();
    
    status = SERVER_STATUS::RUNNING;
    LOG_INFO("Server started successfully");
    
    createServerThreads();
    
    return 0;
}

void Server::banlistAdd(const std::string& username) {
    std::lock_guard<std::mutex> lock(bannedUsersMutex);
    bannedUsers.insert(username);
    saveBanlist();
}

bool Server::banlistRemove(const std::string& username) {
    std::lock_guard<std::mutex> lock(bannedUsersMutex);
    auto it = bannedUsers.find(username);
    if (it == bannedUsers.end()) {
        return false;
    }
    bannedUsers.erase(it);
    saveBanlist();
    return true;
}

bool Server::isBanned(const std::string& username) const {
    std::lock_guard<std::mutex> lock(bannedUsersMutex);
    return bannedUsers.find(username) != bannedUsers.end();
}

void Server::loadBanlist() {
    std::lock_guard<std::mutex> lock(bannedUsersMutex);
    std::ifstream file(Constants::DEFAULT_BANLIST);
    
    if (!file.is_open()) {
        LOG_INFO("No banlist file found, creating a new list");
        return;
    }
    
    std::string username;
    while (std::getline(file, username)) {
        if (!username.empty()) {
            bannedUsers.insert(username);
        }
    }
    
    file.close();
    LOG_INFO("Banlist loaded: " + std::to_string(bannedUsers.size()) + " banned user(s)");
}

void Server::saveBanlist() {
    // Note: Mutex is already locked by banlistAdd
    std::ofstream file(Constants::DEFAULT_BANLIST);
    
    if (!file.is_open()) {
        LOG_ERROR("Cannot open banlist file for writing");
        return;
    }
    
    for (const auto& username : bannedUsers) {
        file << username << "\n";
    }
    
    file.close();
    LOG_DEBUG("Banlist saved: " + std::to_string(bannedUsers.size()) + " user(s)");
}

void Server::stop() {
    if (status == SERVER_STATUS::OFF || status == SERVER_STATUS::STOPPING) {
        return;
    }
    
    LOG_INFO("Stopping server...");
    status = SERVER_STATUS::STOPPING;
    
    {
        std::lock_guard<std::mutex> lock(clientsMutex);
        for (const auto& [username, info] : clients) {
            close(info.socket);
        }
        clients.clear();
    }
    
    if (config.socket > 0) {
        close(config.socket);
        config.socket = 0;
    }
    
    status = SERVER_STATUS::OFF;
    LOG_INFO("Server stopped");
    
    std::exit(0);
}

std::string Server::getUsernameBySocket(int socket) {
    std::lock_guard<std::mutex> lock(clientsMutex);
    for (const auto& [username, info] : clients) {
        if (info.socket == socket) {
            return username;
        }
    }
    return "";
}

int Server::getUserSocket(const std::string& username) {
    std::lock_guard<std::mutex> lock(clientsMutex);
    auto it = clients.find(username);
    return (it != clients.end()) ? it->second.socket : -1;
}

bool Server::isUsernameTaken(const std::string& username) {
    std::lock_guard<std::mutex> lock(clientsMutex);
    return clients.find(username) != clients.end();
}

void Server::registerClient(const std::string& username, int socket) {
    std::lock_guard<std::mutex> lock(clientsMutex);
    clients[username] = ClientInfo(socket);
}

void Server::unregisterClient(const std::string& username) {
    std::lock_guard<std::mutex> lock(clientsMutex);
    clients.erase(username);
}

void Server::updateClientPong(const std::string& username) {
    std::lock_guard<std::mutex> lock(clientsMutex);
    auto it = clients.find(username);
    
    if (it != clients.end()) {
        it->second.lastPong = std::chrono::steady_clock::now();
        it->second.waitingForPong = false;
    }
}



void Server::heartbeatLoop() {
    LOG_INFO("Heartbeat thread started");
    
    while (status == SERVER_STATUS::RUNNING) {
        std::this_thread::sleep_for(
            std::chrono::seconds(Constants::HEARTBEAT_INTERVAL_S)
        );
        
        #ifdef DISABLE_HEARTBEAT
        continue;
        #endif
        
        {
            std::lock_guard<std::mutex> lock(clientsMutex);
            
            for (auto& [username, info] : clients) {
                Network::NetworkStream stream(info.socket);
                (void)stream.send("PING\n");
                info.waitingForPong = true;
                LOG_DEBUG("PING sent to " + username);
            }
        }
        
        // Wait a bit for clients to respond
        std::this_thread::sleep_for(std::chrono::seconds(Constants::HEARTBEAT_CHECK_DELAY_S));
        
        checkClientTimeouts();
    }
    
    LOG_INFO("Heartbeat thread stopped");
}

void Server::checkClientTimeouts() {
    auto now = std::chrono::steady_clock::now();
    std::vector<std::string> timedOutClients;
    
    {
        std::lock_guard<std::mutex> lock(clientsMutex);
        
        for (const auto& [username, info] : clients) {
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                now - info.lastPong
            ).count();
            
            if (elapsed > Constants::HEARTBEAT_TIMEOUT_S) {
                LOG_WARNING("Client timeout: " + username + 
                           " (no response for " + std::to_string(elapsed) + "s)");
                timedOutClients.push_back(username);
            }
        }
    }
    
    for (const auto& username : timedOutClients) {
        int socket = getUserSocket(username);
        if (socket > 0) {
            if (commandHandler) {
                commandHandler->handleDisconnect({}, socket);
            }
        }
    }
}

void Server::initializeConfig(int PORT) {
    config.port = PORT;
    config.address.sin_family = AF_INET;
    config.address.sin_addr.s_addr = INADDR_ANY;
    config.address.sin_port = htons(PORT);
    std::memset(config.address.sin_zero, 0, sizeof(config.address.sin_zero));
}

void Server::initializeCommands() {
    // Helper pour enregistrer une commande
    auto registerCmd = [this](const std::string& name, void (::CommandHandler::*handler)(const std::vector<std::string>&, int)) {
        commands[name] = [handler](Server* srv, const std::vector<std::string>& data, int socket) {
            if (srv->commandHandler) (srv->commandHandler.get()->*handler)(data, socket);
        };
    };
    
    registerCmd("CONNECT",    &::CommandHandler::handleConnect);
    registerCmd("DISCONNECT", &::CommandHandler::handleDisconnect);
    registerCmd("SEND",       &::CommandHandler::handleSendMessage);
    registerCmd("PING",       &::CommandHandler::handlePing);
    registerCmd("PONG",       &::CommandHandler::handlePong);
    registerCmd("LIST_USERS", &::CommandHandler::handleListUsers);
    registerCmd("GET_LOG",    &::CommandHandler::handleGetLog);
    
    LOG_INFO("Commands initialized");
}

void Server::executeCommand(const std::string& commandName, 
                           const std::vector<std::string>& args, 
                           int socket) {
    auto it = commands.find(commandName);
    if (it != commands.end()) {
        std::vector<std::string> parsedData;
        parsedData.reserve(args.size() + 1);
        parsedData.push_back(commandName);
        parsedData.insert(parsedData.end(), args.begin(), args.end());
        
        it->second(this, parsedData, socket);
    } else {
        // Error 1: Unknown command
        LOG_WARNING("Unknown command: " + commandName);
        Network::NetworkStream stream(socket);
        (void)stream.send(Utils::MessageParser::build("ERROR", "Unknown command: " + commandName));
    }
}

void Server::createServerThreads() {
    std::thread acceptThread([this]() {
        acceptClients();
    });
    acceptThread.detach();
    
    std::thread dispatcherThread([this]() {
        if (dispatcher) {
            dispatcher->run();
        }
    });
    dispatcherThread.detach();
    
    #ifndef DISABLE_HEARTBEAT
    std::thread heartbeatThread([this]() {
        heartbeatLoop();
    });
    heartbeatThread.detach();
    LOG_INFO("Heartbeat thread launched");
    #endif
    
    std::thread adminThread([this]() {
        if (adminHandler) {
            adminHandler->commandLoop();
        }
    });
    adminThread.detach();
    LOG_INFO("Admin thread launched - Type /help to see commands");
}

void Server::acceptClients() {
    LOG_INFO("Accept thread started");
    
    while (status == SERVER_STATUS::RUNNING) {
        sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        
        int clientSocket = accept(config.socket, (struct sockaddr*)&clientAddr, &clientLen);
        
        if (clientSocket < 0) {
            if (status == SERVER_STATUS::RUNNING) {
                LOG_ERROR("Failed to accept a client");
            }
            continue;
        }
        
        LOG_INFO("New connection accepted (socket: " + std::to_string(clientSocket) + ")");
        
        if (threadPool) {
            threadPool->enqueue([this, clientSocket]() {
                handleClientMessages(clientSocket);
            });
        } else {
            LOG_ERROR("ThreadPool not initialized");
            close(clientSocket);
        }
    }
    
    LOG_INFO("Accept thread stopped");
}

void Server::handleClientMessages(int clientSocket) {
    Network::NetworkStream stream(clientSocket);
    
    while (status == SERVER_STATUS::RUNNING && stream.isConnected()) {
        auto maybeMessage = stream.receive();
        
        if (!maybeMessage) {
            // Check if client is still registered before attempting disconnect
            std::string username = getUsernameBySocket(clientSocket);
            if (!username.empty() && commandHandler) {
                commandHandler->handleDisconnect({}, clientSocket);
            }
            break;
        }
        
        auto parsed = Utils::MessageParser::parse(*maybeMessage);
        
        if (parsed.isValid) {
            executeCommand(parsed.command, parsed.arguments, clientSocket);
        }
    }
}

int Server::getClientCount() const {
    std::lock_guard<std::mutex> lock(clientsMutex);
    return static_cast<int>(clients.size());
}

std::unordered_map<std::string, int> Server::getAllClients() {
    std::lock_guard<std::mutex> lock(clientsMutex);
    std::unordered_map<std::string, int> result;
    for (const auto& [username, info] : clients) {
        result[username] = info.socket;
    }
    return result;
}

