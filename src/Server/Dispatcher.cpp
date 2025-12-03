#include "Server/Dispatcher.hpp"
#include "Server/Server.hpp"
#include "Utils/Logger.hpp"
#include "Utils/Constants.hpp"
#include "Utils/NetworkStream.hpp"
#include "Utils/MessageParser.hpp"
#include "Utils/Utils.hpp"
#include <thread>

Dispatcher::Dispatcher(Server* attributedServer) : attributedServer(attributedServer) {
    config.startedTimestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    );
    LOG_INFO("Dispatcher created");
}

bool Dispatcher::queueMessage(const Message& msg) {
    std::lock_guard<std::mutex> lock(messagesMutex);
    
    if (messages.size() >= static_cast<size_t>(config.maxStoredMessages)) {
        switch (config.queuePolicy) {
            case QueueFullPolicy::REJECT:
                LOG_WARNING("Queue full - message rejected (policy: REJECT)");
                return false;
                
            case QueueFullPolicy::DROP_OLDEST:
                messages.pop(); 
                LOG_WARNING("Queue full - oldest message dropped (policy: DROP_OLDEST)");
                break;
                
            case QueueFullPolicy::DROP_NEWEST:
                LOG_WARNING("Queue full - new message ignored (policy: DROP_NEWEST)");
                return false;  // Ignore the new message
        }
    }
    
    messages.push(msg);
    cv.notify_one();  // Wake up dispatcher thread
    return true;
}

void Dispatcher::run() {
    LOG_INFO("Dispatcher started");
    
    while (running && attributedServer->getStatus() == SERVER_STATUS::RUNNING) {
        std::unique_lock<std::mutex> lock(messagesMutex);
        
        // Wait for a message to arrive or for the dispatcher to stop
        cv.wait(lock, [this] { 
            return !messages.empty() || !running || attributedServer->getStatus() != SERVER_STATUS::RUNNING;
        });
        
        // Check if we should stop
        if (!running || attributedServer->getStatus() != SERVER_STATUS::RUNNING) {
            break;
        }
        
        // Check if there's a message (could be false if woken up by stop)
        if (messages.empty()) {
            continue;
        }
        
        Message msg = messages.front();
        messages.pop();
        
        lock.unlock();  // Release mutex during processing
        
        // Delay between messages if configured
        if (config.delayBetweenMessages > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(config.delayBetweenMessages));
        }
        
        int recipientSocket = attributedServer->getUserSocket(msg.to);
        
        // Error 6: Recipient just disconnected
        if (recipientSocket <= 0) {
            LOG_WARNING("Recipient not found or disconnected: " + msg.to + " (message from " + msg.from + ")");
            
            // Notify sender that message could not be delivered
            int senderSocket = attributedServer->getUserSocket(msg.from);
            if (senderSocket > 0) {
                Network::NetworkStream senderStream(senderSocket);
                std::string errorMsg = Utils::MessageParser::build(
                    "ERROR", 
                    "Message to '" + msg.to + "' could not be delivered: user disconnected"
                );
                (void)senderStream.send(errorMsg);
            }
            continue;
        }
        
        std::string timestampStr = Utils::timestampToUnixString(msg.timestamp);
        
        Network::NetworkStream stream(recipientSocket);
        std::string formattedMessage = Utils::MessageParser::build(
            "MESSAGE", 
            msg.from, 
            msg.subject, 
            msg.body, 
            timestampStr
        );
        
        if (stream.send(formattedMessage)) {
            attributedServer->incrementMessagesSent();
            LOG_DEBUG("Message dispatched from " + msg.from + " to " + msg.to);
        } else {
            LOG_ERROR("Failed to send message to " + msg.to);
        }
    }
      LOG_INFO("Dispatcher stopped");
}

void Dispatcher::stop() {
    {
        std::lock_guard<std::mutex> lock(messagesMutex);
        running = false;
    }
    cv.notify_all();  // Wake up thread so it can terminate
    LOG_INFO("Dispatcher stop requested");
}