/**
 * @file ServerConfig.hpp
 * @brief Configuration du serveur TCP
 */

#ifndef SERVER_CONFIG_HPP
#define SERVER_CONFIG_HPP

#include <netinet/in.h>

/**
 * @enum SERVER_STATUS
 * @brief États possibles du serveur
 */
enum class SERVER_STATUS {
    OFF,        ///< Serveur arrêté
    STARTING,   ///< En cours de démarrage
    RUNNING,    ///< En fonctionnement
    STOPPING    ///< En cours d'arrêt
};

/**
 * @struct ServerConfig
 * @brief Configuration réseau du serveur
 */
struct ServerConfig {
    int socket;                  ///< File descriptor du socket serveur
    sockaddr_in address;         ///< Adresse du serveur
    int port;                    ///< Port d'écoute
    int max_connections;         ///< Nombre max de connexions simultanées
};

#endif