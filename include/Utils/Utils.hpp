/**
 * @file Utils.hpp
 * @brief Fonctions utilitaires (validation, sanitization, parsing)
 */

#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <vector>
#include <chrono>

namespace Utils {
    /**
     * @brief Découpe une chaîne selon un délimiteur
     * @param s Chaîne à découper
     * @param delimiter Délimiteur
     * @return Vecteur des segments
     */
    std::vector<std::string> split(const std::string& s, const std::string& delimiter);
    
    /**
     * @brief Valide un nom d'utilisateur
     * @param username Nom d'utilisateur
     * @return true si valide (non vide, alphanumérique)
     */
    bool isValidUsername(const std::string& username);
    
    /**
     * @brief Valide un sujet de message
     * @param subject Sujet
     * @return true si valide (non vide, < MAX_SUBJECT_LENGTH)
     */
    bool isValidSubject(const std::string& subject);
    
    /**
     * @brief Valide un corps de message
     * @param body Corps du message
     * @return true si valide (non vide)
     */
    bool isValidBody(const std::string& body);
    
    /**
     * @brief Nettoie une chaîne (supprime caractères invalides)
     * @param input Chaîne à nettoyer
     * @return Chaîne sanitized
     */
    std::string sanitize(const std::string& input);
    
    /**
     * @brief Convertit un timestamp en string Unix (secondes depuis epoch)
     * @param tp Timestamp à convertir
     * @return String représentant le timestamp en secondes
     */
    std::string timestampToUnixString(const std::chrono::system_clock::time_point& tp);
    
    /**
     * @brief Parse un string Unix vers timestamp
     * @param str String représentant le timestamp en secondes
     * @return Timestamp
     */
    std::chrono::system_clock::time_point unixStringToTimestamp(const std::string& str);
    
    /**
     * @brief Formate un timestamp en string lisible
     * @param tp Timestamp à formater
     * @return String formaté (sans newline final)
     */
    std::string formatTimestamp(const std::chrono::system_clock::time_point& tp);
}

#endif