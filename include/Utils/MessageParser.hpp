/**
 * @file MessageParser.hpp
 * @brief Message parser with delimited protocol
 */

#ifndef MESSAGE_PARSER_HPP
#define MESSAGE_PARSER_HPP

#include <string>
#include <vector>

namespace Utils {

/**
 * @class MessageParser
 * @brief Parses and builds messages according to protocol
 */
class MessageParser {
public:
    /**
     * @struct ParsedMessage
     * @brief Result of message parsing
     */
    struct ParsedMessage {
        std::string command;
        std::vector<std::string> arguments;
        bool isValid = false;
        
        size_t argCount() const { return arguments.size(); }
        
        std::string arg(size_t index) const {
            return index < arguments.size() ? arguments[index] : "";
        }
    };
    
    /**
     * @brief Parses a raw message
     * @param rawMessage Message to parse
     * @return Parsed message
     */
    static ParsedMessage parse(const std::string& rawMessage);
    
    /**
     * @brief Builds a formatted message
     * @param command Command
     * @param args Arguments
     * @return Formatted message
     */
    static std::string build(const std::string& command, 
                            const std::vector<std::string>& args = {});
    
    /**
     * @brief Construit un message avec arguments variadiques
     */
    template<typename... Args>
    static std::string build(const std::string& command, Args... args) {
        return build(command, std::vector<std::string>{args...});
    }
};

} // namespace Utils

#endif