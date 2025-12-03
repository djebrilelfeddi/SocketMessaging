#include "Utils/Utils.hpp"
#include "Utils/Constants.hpp"
#include "Utils/RuntimeConfig.hpp"
#include <algorithm>
#include <cctype>

namespace Utils {

    bool isValidUsername(const std::string& username) {
        if (username.empty()) {
            return false;
        }
        
        // Utiliser la config runtime pour la longueur max
        auto maxLength = RuntimeConfig::getInstance().getInt("MAX_USERNAME_LENGTH");
        size_t limit = maxLength.has_value() ? static_cast<size_t>(maxLength.value()) : Constants::MAX_USERNAME_LENGTH;
        
        if (username.length() > limit) {
            return false;
        }
        
        return std::all_of(username.begin(), username.end(), [](char c) {
            return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
        });
    }

    bool isValidSubject(const std::string& subject) {
        if (subject.empty()) {
            return false;
        }
        
        // Utiliser la config runtime pour la longueur max
        auto maxLength = RuntimeConfig::getInstance().getInt("MAX_SUBJECT_LENGTH");
        size_t limit = maxLength.has_value() ? static_cast<size_t>(maxLength.value()) : Constants::MAX_SUBJECT_LENGTH;
        
        return subject.length() <= limit;
    }

    bool isValidBody(const std::string& body) {
        return !body.empty();
    }

    std::string sanitize(const std::string& input) {
        std::string result;
        result.reserve(input.length());
        
        for (char c : input) {
            if (std::iscntrl(static_cast<unsigned char>(c)) && c != '\n' && c != '\t') {
                result += ' ';
            } else {
                result += c;
            }
        }
        
        return result;
    }

    std::vector<std::string> split(const std::string& s, const std::string& delimiter) {
        std::vector<std::string> tokens;
        
        if (s.empty() || delimiter.empty()) {
            return tokens;
        }
        
        size_t start = 0;
        size_t end = s.find(delimiter);
        
        while (end != std::string::npos) {
            tokens.push_back(s.substr(start, end - start));
            start = end + delimiter.length();
            end = s.find(delimiter, start);
        }
        
        tokens.push_back(s.substr(start));
        return tokens;
    }

    std::string timestampToUnixString(const std::chrono::system_clock::time_point& tp) {
        auto time_t_timestamp = std::chrono::system_clock::to_time_t(tp);
        return std::to_string(time_t_timestamp);
    }

    std::chrono::system_clock::time_point unixStringToTimestamp(const std::string& str) {
        try {
            std::time_t time_t_val = std::stoll(str);
            return std::chrono::system_clock::from_time_t(time_t_val);
        } catch (...) {
            return std::chrono::system_clock::now();
        }
    }

    std::string formatTimestamp(const std::chrono::system_clock::time_point& tp) {
        auto time_t_timestamp = std::chrono::system_clock::to_time_t(tp);
        std::string timeStr = std::ctime(&time_t_timestamp);
        if (!timeStr.empty() && timeStr.back() == '\n') {
            timeStr.pop_back();
        }
        return timeStr;
    }

} 