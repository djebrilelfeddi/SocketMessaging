/**
 * @file ClientUI.cpp
 * @brief Client user interface (all display logic is here)
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
    std::cout << "   MESSAGING CLIENT\n";
    std::cout << "========================================" << Color::RESET << "\n";
    std::cout << "Server: " << serverIp << ":" << serverPort << "\n\n";
}

void ClientUI::print(const std::string& msg) {
    std::cout << msg;
}

void ClientUI::printError(const std::string& msg) {
    std::cout << Color::BRIGHT_RED << "[ERROR] " << Color::RESET << msg << "\n";
}

void ClientUI::printSuccess(const std::string& msg) {
    std::cout << Color::BRIGHT_GREEN << "[OK] " << Color::RESET << msg << "\n";
}

void ClientUI::printMenu() {
    std::cout << "\n" << Color::BRIGHT_CYAN << client.getCurrentUsername() << Color::RESET 
              << " - Connected to " << Color::CYAN << serverIp << ":" << serverPort << Color::RESET << "\n";
    std::cout << "\n" << Color::BRIGHT_MAGENTA << "=== MENU ===" << Color::RESET << "\n";
    std::cout << "1. Send a message\n";
    std::cout << "2. Unread messages\n";
    std::cout << "3. Read a message\n";
    std::cout << "4. Online users\n";
    std::cout << "5. Broadcast\n";
    std::cout << "6. Server logs\n";
    std::cout << Color::BRIGHT_RED << "7. Quit" << Color::RESET << "\n";
    std::cout << Color::YELLOW << "$ " << Color::RESET;
}

void ClientUI::promptAndWait() {
    std::cout << "\n" << Color::GRAY << "Press Enter to continue..." << Color::RESET;
    std::cin.get();
    clearScreen();
}

void ClientUI::printUserList(const std::string& userListData) {
    auto users = Utils::split(userListData, ",");
    std::cout << Color::BRIGHT_GREEN << "=== Users (" << users.size() << ") ===" << Color::RESET << "\n";
    for (const auto& user : users) {
        if (user == client.getCurrentUsername()) {
            std::cout << "  " << Color::BRIGHT_YELLOW << "* " << user << " (You)" << Color::RESET << "\n";
        } else {
            std::cout << "  " << user << "\n";
        }
    }
}

// === Connexion ===

bool ClientUI::promptAndConnect() {
    printHeader();
    
    std::string username;
    std::cout << Color::YELLOW << "Username: " << Color::RESET;
    std::getline(std::cin, username);
    
    if (!Utils::isValidUsername(username)) {
        printError("Invalid name (alphanumeric and underscore, max 16 characters)");
        return false;
    }
    
    std::string errorMsg;
    if (!client.connect(username, errorMsg)) {
        printError(errorMsg.empty() ? "Connection failed" : errorMsg);
        return false;
    }
    
    printSuccess("Connected!");
    return true;
}

// === Commandes ===

void ClientUI::cmdSendMessage(bool broadcast) {
    auto* handler = client.getMessageHandler();
    if (!handler) return;
    
    std::string to, subject, body;
    
    if (broadcast) {
        to = "all";
        std::cout << Color::BRIGHT_MAGENTA << "Sending to all" << Color::RESET << "\n";
    } else {
        std::cout << "Recipient: ";
        std::getline(std::cin, to);
    }
    
    std::cout << "Subject: ";
    std::getline(std::cin, subject);
    
    std::cout << "Message: ";
    std::getline(std::cin, body);
    
    if (!handler->sendMessage(to, subject, body)) {
        printError("Failed to send");
    }
}

void ClientUI::cmdListUnread() {
    auto* handler = client.getMessageHandler();
    if (!handler) return;
    
    auto messages = handler->getUnreadMessages();
    
    if (messages.empty()) {
        std::cout << Color::YELLOW << "No unread messages." << Color::RESET << "\n";
        return;
    }
    
    std::cout << "\n" << Color::BRIGHT_MAGENTA << "=== Unread messages (" << messages.size() << ") ===" << Color::RESET << "\n";
    for (const auto& msg : messages) {
        std::cout << Color::CYAN << "[" << msg.index << "]" << Color::RESET << " From: " << std::setw(12) << msg.from << " | " << msg.subject << "\n";
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
        printError("Message not found");
        return;
    }
    
    std::cout << "\n" << Color::BRIGHT_CYAN << "=== Message #" << index << " ===" << Color::RESET << "\n";
    std::cout << "From: " << msg.from << "\n";
    std::cout << "Subject: " << msg.subject << "\n";
    std::cout << "Date: " << Utils::formatTimestamp(msg.timestamp) << "\n";
    std::cout << "---\n" << msg.body << "\n";
    
    std::cout << "\n[r] Reply | [Enter] Back: ";
    std::string action;
    std::getline(std::cin, action);
    
    if (action == "r" || action == "R") {
        std::cout << "Reply: ";
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
        // Wait for server response
        waitForResponse(ServerEvent::USERS);
    }
}

void ClientUI::cmdGetLog() {
    auto* handler = client.getMessageHandler();
    if (handler) {
        handler->sendCommand(Utils::MessageParser::build("GET_LOG"));
        // Wait for server response
        waitForResponse(ServerEvent::LOG);
    }
}

// === Event handling ===

void ClientUI::waitForResponse(ServerEvent expectedType, int timeoutMs) {
    // Temporarily accept direct events for display
    inCommand.store(false);
    
    auto start = std::chrono::steady_clock::now();
    while (true) {
        // Check timeout
        auto elapsed = std::chrono::steady_clock::now() - start;
        if (std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() > timeoutMs) {
            break;
        }
        
        // Check if we received the expected response
        {
            std::lock_guard<std::mutex> lock(eventQueueMutex);
            if (lastReceivedEvent == expectedType) {
                lastReceivedEvent = ServerEvent::NONE;
                break;
            }
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    
    // Return to command mode
    inCommand.store(true);
}

void ClientUI::onServerEvent(const ServerEventData& event) {
    // Always queue if waiting for user input
    if (inCommand.load()) {
        std::lock_guard<std::mutex> lock(eventQueueMutex);
        if (event.type != ServerEvent::OK) pendingEvents.push(event);
        return;
    }
    
    // Immediate display only when at menu (after menu display)
    displayEvent(event);
    
    // Record type for waitForResponse
    {
        std::lock_guard<std::mutex> lock(eventQueueMutex);
        lastReceivedEvent = event.type;
    }
}

void ClientUI::displayEvent(const ServerEventData& event) {
    switch (event.type) {
        case ServerEvent::MESSAGE:
            std::cout << "\n" << Color::BRIGHT_YELLOW << "New Message: " 
                      << event.args[0] << " - " << event.args[1] << Color::RESET << "\n$ " << std::flush;
            break;
        case ServerEvent::OK:
            std::cout << "\n" << Color::BRIGHT_GREEN << "[OK] " << Color::RESET << event.data << "\n$ " << std::flush;
            break;
        case ServerEvent::ERROR_MSG:
            std::cout << "\n" << Color::BRIGHT_RED << "[ERROR] " << Color::RESET << event.data << "\n$ " << std::flush;
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
    
    std::cout << "\n" << Color::GRAY << "--- While you were busy ---" << Color::RESET << "\n";
    while (!pendingEvents.empty()) {
        auto& event = pendingEvents.front();
        switch (event.type) {
            case ServerEvent::MESSAGE:
                std::cout << Color::BRIGHT_GREEN << "New message from " 
                          << event.args[0] << " - " << event.args[1] << Color::RESET << "\n";
                break;
            case ServerEvent::ERROR_MSG:
                std::cout << Color::BRIGHT_RED << "[ERROR] " << Color::RESET << event.data << "\n";
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

// === Main loop ===

bool ClientUI::run() {
    if (!promptAndConnect()) {
        return false;
    }
    
    clearScreen();
    
    // Start listening with callback to UI
    client.startListening([this](const ServerEventData& event) {
        onServerEvent(event);
    });
    
    bool running = true;
    while (running) {
        // Display menu then release to receive events
        printMenu();
        inCommand.store(false);
        
        std::string choice;
        std::getline(std::cin, choice);
        
        // Block events during processing
        inCommand.store(true);
        clearScreen();
        
        if (choice == "7") {
            std::cout << Color::YELLOW << "Disconnecting..." << Color::RESET << "\n";
            running = false;
        } else {
            auto it = commands.find(choice);
            if (it != commands.end()) {
                it->second();
                promptAndWait();
                displayPendingEvents();
            } else {
                printError("Invalid choice");
                promptAndWait();
            }
        }
    }
    
    client.disconnect();
    return true;
}
