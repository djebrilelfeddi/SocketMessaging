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
    LOG_INFO("Dispatcher créé");
}

bool Dispatcher::queueMessage(const Message& msg) {
    std::lock_guard<std::mutex> lock(messagesMutex);
    
    if (messages.size() >= static_cast<size_t>(config.maxStoredMessages)) {
        switch (config.queuePolicy) {
            case QueueFullPolicy::REJECT:
                LOG_WARNING("Queue pleine - message rejeté (policy: REJECT)");
                return false;
                
            case QueueFullPolicy::DROP_OLDEST:
                messages.pop(); 
                LOG_WARNING("Queue pleine - message le plus ancien supprimé (policy: DROP_OLDEST)");
                break;
                
            case QueueFullPolicy::DROP_NEWEST:
                LOG_WARNING("Queue pleine - nouveau message ignoré (policy: DROP_NEWEST)");
                return false;  // Ignore le nouveau message
        }
    }
    
    messages.push(msg);
    cv.notify_one();  // Réveille le thread du dispatcher
    return true;
}

void Dispatcher::run() {
    LOG_INFO("Dispatcher démarré");
    
    while (running && attributedServer->getStatus() == SERVER_STATUS::RUNNING) {
        std::unique_lock<std::mutex> lock(messagesMutex);
        
        // Attend qu'un message arrive ou que le dispatcher s'arrête
        cv.wait(lock, [this] { 
            return !messages.empty() || !running || attributedServer->getStatus() != SERVER_STATUS::RUNNING;
        });
        
        // Vérifie si on doit s'arrêter
        if (!running || attributedServer->getStatus() != SERVER_STATUS::RUNNING) {
            break;
        }
        
        // Vérifie s'il y a un message (pourrait être faux si réveillé par stop)
        if (messages.empty()) {
            continue;
        }
        
        Message msg = messages.front();
        messages.pop();
        
        lock.unlock();  // Libère le mutex pendant le traitement
        
        // Délai entre messages si configuré
        if (config.delayBetweenMessages > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(config.delayBetweenMessages));
        }
        
        int recipientSocket = attributedServer->getUserSocket(msg.to);
        
        // Erreur 6: Le destinataire vient de se déconnecter
        if (recipientSocket <= 0) {
            LOG_WARNING("Destinataire non trouvé ou déconnecté: " + msg.to + " (message de " + msg.from + ")");
            
            // Notifier l'expéditeur que le message n'a pas pu être délivré
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
            LOG_DEBUG("Message dispatché de " + msg.from + " vers " + msg.to);
        } else {
            LOG_ERROR("Échec d'envoi du message à " + msg.to);
        }
    }
      LOG_INFO("Dispatcher arrêté");
}

void Dispatcher::stop() {
    {
        std::lock_guard<std::mutex> lock(messagesMutex);
        running = false;
    }
    cv.notify_all();  // Réveille le thread pour qu'il se termine
    LOG_INFO("Arrêt du dispatcher demandé");
}