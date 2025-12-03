/**
 * @file ServerConfig.hpp
 * @brief TCP server configuration
 */

#ifndef SERVER_CONFIG_HPP
#define SERVER_CONFIG_HPP

#include <netinet/in.h>

/**
 * @enum SERVER_STATUS
 * @brief Possible server states
 */
enum class SERVER_STATUS {
    OFF,        ///< Server stopped
    STARTING,   ///< Starting up
    RUNNING,    ///< Running
    STOPPING    ///< Shutting down
};

/**
 * @struct ServerConfig
 * @brief Server network configuration
 */
struct ServerConfig {
    int socket;                  ///< Server socket file descriptor
    sockaddr_in address;         ///< Server address
    int port;                    ///< Listening port
    int max_connections;         ///< Max simultaneous connections
};

#endif