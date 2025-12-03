#include "Utils/MessageParser.hpp"
#include "Utils/Utils.hpp"
#include "Utils/Constants.hpp"
#include <sstream>

namespace Utils {

MessageParser::ParsedMessage MessageParser::parse(const std::string& rawMessage) {
    ParsedMessage result;
    
    if (rawMessage.empty()) {
        return result;
    }
    
    std::string cleaned = rawMessage;
    if (cleaned.back() == '\n') {
        cleaned.pop_back();
    }
    
    auto parts = Utils::split(cleaned, Constants::MESSAGE_DELIMITER);
    
    if (parts.empty()) {
        return result;
    }
    
    result.command = parts[0];
    result.arguments = std::vector<std::string>(parts.begin() + 1, parts.end());
    result.isValid = true;
    
    return result;
}

std::string MessageParser::build(const std::string& command, 
                                 const std::vector<std::string>& args) {
    std::ostringstream oss;
    oss << command;
    
    for (const auto& arg : args) {
        oss << Constants::MESSAGE_DELIMITER << arg;
    }
    
    oss << "\n";
    return oss.str();
}

}
