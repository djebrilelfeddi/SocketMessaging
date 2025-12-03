/**
 * @file DispatcherConfig.hpp
 * @brief Configuration du dispatcher de messages
 */

#ifndef DISPATCHER_CONFIG_HPP
#define DISPATCHER_CONFIG_HPP

#include <chrono>

/**
 * @enum QueueFullPolicy
 * @brief Politique de gestion de la queue pleine
 */
enum class QueueFullPolicy {
    REJECT,      ///< Rejeter les nouveaux messages
    DROP_OLDEST, ///< Supprimer les plus anciens
    DROP_NEWEST  ///< Supprimer les plus récents
};

/**
 * @struct DispatcherConfig
 * @brief Configuration du dispatcher
 */
struct DispatcherConfig {
    int delayBetweenMessages = 10;              ///< Délai entre messages (ms)
    std::chrono::milliseconds startedTimestamp; ///< Timestamp de démarrage
    int maxStoredMessages = 10000;              ///< Taille max queue
    QueueFullPolicy queuePolicy = QueueFullPolicy::REJECT; ///< Politique queue pleine
};

#endif