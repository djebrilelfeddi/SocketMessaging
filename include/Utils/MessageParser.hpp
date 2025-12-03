/**
 * @file MessageParser.hpp
 * @brief Parseur de messages avec protocole délimité
 */

#ifndef MESSAGE_PARSER_HPP
#define MESSAGE_PARSER_HPP

#include <string>
#include <vector>

namespace Utils {

/**
 * @class MessageParser
 * @brief Parse et construit des messages selon le protocole
 */
class MessageParser {
public:
    /**
     * @struct ParsedMessage
     * @brief Résultat du parsing d'un message
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
     * @brief Parse un message brut
     * @param rawMessage Message à parser
     * @return Message parsé
     */
    static ParsedMessage parse(const std::string& rawMessage);
    
    /**
     * @brief Construit un message formaté
     * @param command Commande
     * @param args Arguments
     * @return Message formaté
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