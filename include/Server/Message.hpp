/**
 * @file Message.hpp
 * @brief Structure représentant un message entre utilisateurs
 */

#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <string>
#include <chrono>

/**
 * @struct Message
 * @brief Message envoyé entre utilisateurs du serveur
 */
struct Message {
    std::string from;      ///< Expéditeur
    std::string to;        ///< Destinataire
    std::string subject;   ///< Sujet du message
    std::string body;      ///< Corps du message
    std::chrono::system_clock::time_point timestamp; ///< Date d'envoi
    
    /**
     * @brief Constructeur par défaut (timestamp = now)
     */
    Message() : timestamp(std::chrono::system_clock::now()) {}
};

#endif