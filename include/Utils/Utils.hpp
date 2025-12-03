/**
 * @file Utils.hpp
 * @brief Utility functions (validation, sanitization, parsing)
 */

#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <vector>
#include <chrono>

namespace Utils {
    /**
     * @brief Splits a string by delimiter
     * @param s String to split
     * @param delimiter Delimiter
     * @return Vector of segments
     */
    std::vector<std::string> split(const std::string& s, const std::string& delimiter);
    
    /**
     * @brief Validates a username
     * @param username Username
     * @return true if valid (non-empty, alphanumeric)
     */
    bool isValidUsername(const std::string& username);
    
    /**
     * @brief Validates a message subject
     * @param subject Subject
     * @return true if valid (non-empty, < MAX_SUBJECT_LENGTH)
     */
    bool isValidSubject(const std::string& subject);
    
    /**
     * @brief Validates a message body
     * @param body Message body
     * @return true if valid (non-empty)
     */
    bool isValidBody(const std::string& body);
    
    /**
     * @brief Cleans a string (removes invalid characters)
     * @param input String to clean
     * @return Sanitized string
     */
    std::string sanitize(const std::string& input);
    
    /**
     * @brief Converts a timestamp to Unix string (seconds since epoch)
     * @param tp Timestamp to convert
     * @return String representing the timestamp in seconds
     */
    std::string timestampToUnixString(const std::chrono::system_clock::time_point& tp);
    
    /**
     * @brief Parses a Unix string to timestamp
     * @param str String representing the timestamp in seconds
     * @return Timestamp
     */
    std::chrono::system_clock::time_point unixStringToTimestamp(const std::string& str);
    
    /**
     * @brief Formats a timestamp to readable string
     * @param tp Timestamp to format
     * @return Formatted string (without trailing newline)
     */
    std::string formatTimestamp(const std::chrono::system_clock::time_point& tp);
}

#endif