/**
 * @file Dispatcher.hpp
 * @brief Dispatcher de messages avec queue et traitement par blocs
 */

#ifndef DISPATCHER_HPP
#define DISPATCHER_HPP

#include "Server/Message.hpp"
#include "Server/DispatcherConfig.hpp"
#include <queue>
#include <memory>
#include <mutex>
#include <condition_variable>

class Server;

/**
 * @class Dispatcher
 * @brief Gère la file d'attente et l'envoi des messages
 * 
 * Thread-safe. Traite les messages par blocs pour optimiser les performances.
 */
class Dispatcher {
public:
    Dispatcher() = delete;
    
    /**
     * @brief Constructeur
     * @param attributedServer Pointeur vers le serveur
     */
    explicit Dispatcher(Server* attributedServer);
    
    /**
     * @brief Ajoute un message à la queue
     * @param msg Message à envoyer
     * @return true si ajouté, false si queue pleine
     */
    bool queueMessage(const Message& msg);
    
    /**
     * @brief Boucle principale de traitement (bloquante)
     */
    void run();
      /**
     * @brief Arrête le dispatcher proprement
     */
    void stop();

private:
    std::queue<Message> messages;
    Server* attributedServer;
    DispatcherConfig config;
    std::mutex messagesMutex;
    std::condition_variable cv;
    bool running = true;
};

#endif