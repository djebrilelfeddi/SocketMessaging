/**
 * @file Message.hpp
 * @brief Structure representing a message between users
 */

#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <string>
#include <chrono>

/**
 * @struct Message
 * @brief Message sent between server users
 */
struct Message {
    std::string from;      ///< Sender
    std::string to;        ///< Recipient
    std::string subject;   ///< Message subject
    std::string body;      ///< Message body
    std::chrono::system_clock::time_point timestamp; ///< Send date
    
    /**
     * @brief Default constructor (timestamp = now)
     */
    Message() : timestamp(std::chrono::system_clock::now()) {}
};

#endif