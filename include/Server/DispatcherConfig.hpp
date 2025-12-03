/**
 * @file DispatcherConfig.hpp
 * @brief Message dispatcher configuration
 */

#ifndef DISPATCHER_CONFIG_HPP
#define DISPATCHER_CONFIG_HPP

#include <chrono>

/**
 * @enum QueueFullPolicy
 * @brief Policy for handling full queue
 */
enum class QueueFullPolicy {
    REJECT,      ///< Reject new messages
    DROP_OLDEST, ///< Remove oldest messages
    DROP_NEWEST  ///< Remove newest messages
};

/**
 * @struct DispatcherConfig
 * @brief Dispatcher configuration
 */
struct DispatcherConfig {
    int delayBetweenMessages = 10;              ///< Delay between messages (ms)
    std::chrono::milliseconds startedTimestamp; ///< Startup timestamp
    int maxStoredMessages = 10000;              ///< Max queue size
    QueueFullPolicy queuePolicy = QueueFullPolicy::REJECT; ///< Full queue policy
};

#endif