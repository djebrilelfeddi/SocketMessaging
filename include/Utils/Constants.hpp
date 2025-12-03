/**
 * @file Constants.hpp
 * @brief Constantes globales du projet (buffers, timeouts, limites)
 */

#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include <cstddef>
#include <chrono>
#include <string>

/**
 * @namespace Constants
 * @brief Constantes centralisées du projet
 */
namespace Constants {
    constexpr size_t BUFFER_SIZE = 4096;                ///< Taille buffer réseau
    constexpr size_t MAX_MESSAGE_SIZE = 10 * 1024 * 1024; ///< Taille max message (10MB)
    constexpr size_t MAX_QUEUE_SIZE = 1000;              ///< Taille max queue dispatcher
    
    constexpr int HEARTBEAT_INTERVAL_S = 30;            ///< Intervalle heartbeat (s)
    constexpr int HEARTBEAT_CHECK_DELAY_S = 5;          ///< Délai d'attente après PING avant vérification (s)
    constexpr int HEARTBEAT_TIMEOUT_S = 90;             ///< Timeout avant timeout client (s)
    constexpr int CLIENT_TIMEOUT_S = 120;                ///< Timeout inactivité client (s)
    constexpr int DISPATCHER_SLEEP_MS = 1;               ///< Sleep dispatcher (ms)
    constexpr int MAIN_LOOP_SLEEP_S = 1;                 ///< Sleep main loop (s)
    
    constexpr bool AUTO_STOP_WHEN_NO_CLIENTS = false;    ///< Arrêter le serveur automatiquement quand plus de clients
    
    constexpr size_t THREAD_POOL_SIZE = 12;              ///< Nombre de workers ThreadPool
    
    constexpr int DEFAULT_PORT = 8080;                   ///< Port par défaut
    constexpr int MAX_PENDING_CONNECTIONS = 10;          ///< Max connexions en attente
    constexpr int SOCKET_ERROR = -1;                     ///< Valeur erreur socket
    
    constexpr size_t MAX_USERNAME_LENGTH = 32;           ///< Longueur max username
    constexpr size_t MAX_SUBJECT_LENGTH = 100;           ///< Longueur max subject
    
    // Limites de validation pour RuntimeConfig
    constexpr int MIN_USERNAME_LENGTH = 3;               ///< Longueur min username
    constexpr int MAX_USERNAME_LENGTH_LIMIT = 100;       ///< Longueur max username (limite)
    constexpr int MIN_SUBJECT_LENGTH = 10;               ///< Longueur min subject
    constexpr int MAX_SUBJECT_LENGTH_LIMIT = 500;        ///< Longueur max subject (limite)
    constexpr int MIN_HEARTBEAT_INTERVAL_S = 5;          ///< Intervalle min heartbeat
    constexpr int MIN_HEARTBEAT_TIMEOUT_S = 10;          ///< Timeout min heartbeat
    
    constexpr const char* MESSAGE_DELIMITER = ";";       ///< Délimiteur de champs dans messages
    
    const std::string DEFAULT_SERVER_LOG = "server.log"; ///< Fichier log serveur
    const std::string DEFAULT_CLIENT_LOG = "client.log"; ///< Fichier log client
    const std::string DEFAULT_BANLIST = "banlist";       ///< Fichier des clinets bannis
    
    constexpr size_t LENGTH_PREFIX_SIZE = 4;             ///< Taille préfixe longueur (bytes)
}

#endif