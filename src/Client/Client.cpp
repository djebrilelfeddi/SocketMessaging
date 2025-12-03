/**
 * @file Client.cpp
 * @brief Client TCP simplifié
 */

#include "Client/Client.hpp"
#include "Utils/Logger.hpp"
#include "Utils/NetworkStream.hpp"
#include "Utils/MessageParser.hpp"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

Client::Client(const std::string& serverAddress, int serverPort)
    : serverAddress(serverAddress), serverPort(serverPort) {
    LOG_INFO("Client créé pour " + serverAddress + ":" + std::to_string(serverPort));
}

Client::~Client() {
    disconnect();
}

bool Client::connect(const std::string& username) {
    std::string ignored;
    return connect(username, ignored);
}

bool Client::connect(const std::string& username, std::string& outError) {
    if (isConnected) {
        outError = "Déjà connecté";
        return false;
    }
    
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        outError = "Échec création socket";
        LOG_ERROR(outError);
        return false;
    }
    clientSocket = Socket(fd);
    
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);
    
    if (inet_pton(AF_INET, serverAddress.c_str(), &serverAddr.sin_addr) <= 0) {
        outError = "Adresse invalide: " + serverAddress;
        LOG_ERROR(outError);
        clientSocket.close();
        return false;
    }
    
    if (::connect(clientSocket.get(), (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        outError = "Impossible de se connecter au serveur";
        LOG_ERROR(outError);
        clientSocket.close();
        return false;
    }
    
    Network::NetworkStream stream(clientSocket.get());
    if (!stream.send(Utils::MessageParser::build("CONNECT", username))) {
        outError = "Échec envoi requête de connexion";
        clientSocket.close();
        return false;
    }
    
    auto response = stream.receive();
    if (!response) {
        outError = "Pas de réponse du serveur";
        clientSocket.close();
        return false;
    }
    
    auto parsed = Utils::MessageParser::parse(*response);
    if (!parsed.isValid || parsed.command != "OK") {
        outError = (parsed.isValid && parsed.command == "ERROR" && parsed.argCount() > 0) ? parsed.arg(0) : "Connexion refusée";
        clientSocket.close();
        return false;
    }
    
    this->username = username;
    isConnected = true;
    messageHandler = std::make_unique<MessageHandler>(clientSocket.get());
    messageHandler->setCurrentUsername(username);
    
    LOG_CONNECT("Connecté en tant que " + username);
    return true;
}

void Client::disconnect() {
    if (!isConnected) return;
    
    if (clientSocket.isValid()) {
        Network::NetworkStream stream(clientSocket.get());
        (void)stream.send(Utils::MessageParser::build("DISCONNECT"));
    }
    
    if (listenerThread.joinable()) {
        listenerThread.join();
    }
    
    clientSocket.close();
    isConnected = false;
    messageHandler.reset();
    LOG_DISCONNECT("Déconnecté");
}

void Client::startListening(MessageHandler::EventCallback onEvent) {
    if (!isConnected || !messageHandler) return;
    
    //LOG_INFO("Démarrage écoute");
    listenerThread = std::thread([this, onEvent]() {
        messageHandler->listen(onEvent);
    });
}

MessageHandler* Client::getMessageHandler() const {
    return messageHandler.get();
}
