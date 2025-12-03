/**
 * @file Constants.hpp
 * @brief Global project constants (buffers, timeouts, limits)
 */

#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include <cstddef>
#include <chrono>
#include <string>

/**
 * @namespace Constants
 * @brief Centralized project constants
 */
namespace Constants {
    constexpr size_t BUFFER_SIZE = 4096;                ///< Network buffer size
    constexpr size_t MAX_MESSAGE_SIZE = 10 * 1024 * 1024; ///< Max message size (10MB)
    constexpr size_t MAX_QUEUE_SIZE = 1000;              ///< Max dispatcher queue size
    
    constexpr int HEARTBEAT_INTERVAL_S = 30;            ///< Heartbeat interval (s)
    constexpr int HEARTBEAT_CHECK_DELAY_S = 5;          ///< Delay after PING before checking (s)
    constexpr int HEARTBEAT_TIMEOUT_S = 90;             ///< Timeout before client timeout (s)
    constexpr int CLIENT_TIMEOUT_S = 120;                ///< Client inactivity timeout (s)
    constexpr int DISPATCHER_SLEEP_MS = 1;               ///< Dispatcher sleep (ms)
    constexpr int MAIN_LOOP_SLEEP_S = 1;                 ///< Main loop sleep (s)
    
    constexpr bool AUTO_STOP_WHEN_NO_CLIENTS = false;    ///< Auto stop server when no clients
    
    constexpr size_t THREAD_POOL_SIZE = 12;              ///< ThreadPool workers count
    
    constexpr int DEFAULT_PORT = 8080;                   ///< Default port
    constexpr int MAX_PENDING_CONNECTIONS = 10;          ///< Max pending connections
    constexpr int SOCKET_ERROR = -1;                     ///< Socket error value
    
    constexpr size_t MAX_USERNAME_LENGTH = 32;           ///< Max username length
    constexpr size_t MAX_SUBJECT_LENGTH = 100;           ///< Max subject length
    
    // Validation limits for RuntimeConfig
    constexpr int MIN_USERNAME_LENGTH = 3;               ///< Min username length
    constexpr int MAX_USERNAME_LENGTH_LIMIT = 100;       ///< Max username length (limit)
    constexpr int MIN_SUBJECT_LENGTH = 10;               ///< Min subject length
    constexpr int MAX_SUBJECT_LENGTH_LIMIT = 500;        ///< Max subject length (limit)
    constexpr int MIN_HEARTBEAT_INTERVAL_S = 5;          ///< Min heartbeat interval
    constexpr int MIN_HEARTBEAT_TIMEOUT_S = 10;          ///< Min heartbeat timeout
    
    constexpr const char* MESSAGE_DELIMITER = ";";       ///< Message field delimiter
    
    const std::string DEFAULT_SERVER_LOG = "server.log"; ///< Server log file
    const std::string DEFAULT_CLIENT_LOG = "client.log"; ///< Client log file
    const std::string DEFAULT_BANLIST = "banlist";       ///< Banned clients file
    
    constexpr size_t LENGTH_PREFIX_SIZE = 4;             ///< Length prefix size (bytes)
}

#endif