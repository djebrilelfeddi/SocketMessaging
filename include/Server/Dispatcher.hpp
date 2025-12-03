/**
 * @file Dispatcher.hpp
 * @brief Message dispatcher with queue and batch processing
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
 * @brief Manages message queue and delivery
 * 
 * Thread-safe. Processes messages in batches for optimized performance.
 */
class Dispatcher {
public:
    Dispatcher() = delete;
    
    /**
     * @brief Constructor
     * @param attributedServer Pointer to the server
     */
    explicit Dispatcher(Server* attributedServer);
    
    /**
     * @brief Adds a message to the queue
     * @param msg Message to send
     * @return true if added, false if queue full
     */
    bool queueMessage(const Message& msg);
    
    /**
     * @brief Main processing loop (blocking)
     */
    void run();
      /**
     * @brief Stops the dispatcher gracefully
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