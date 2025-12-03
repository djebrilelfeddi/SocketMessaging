/**
 * @file RuntimeConfig.hpp
 * @brief Configuration runtime modifiable via commandes
 */

#ifndef RUNTIME_CONFIG_HPP
#define RUNTIME_CONFIG_HPP

#include <string>
#include <unordered_map>
#include <mutex>
#include <optional>
#include <climits>

/**
 * @enum ConfigType
 * @brief Type de valeur de configuration
 */
enum class ConfigType { INT, BOOL };

/**
 * @struct ConfigDef
 * @brief Définition d'une configuration avec ses contraintes
 */
struct ConfigDef {
    ConfigType type;
    std::string defaultValue;
    int minValue = 0;      // Pour INT uniquement
    int maxValue = INT_MAX; // Pour INT uniquement
};

/**
 * @class RuntimeConfig
 * @brief Singleton pour gérer les configurations modifiables à l'exécution
 */
class RuntimeConfig {
public:
    static RuntimeConfig& getInstance() {
        static RuntimeConfig instance;
        return instance;
    }

    RuntimeConfig(const RuntimeConfig&) = delete;
    RuntimeConfig& operator=(const RuntimeConfig&) = delete;

    /**
     * @brief Définit une valeur de configuration
     * @param key Nom de la constante
     * @param value Nouvelle valeur (string)
     * @return true si la modification a réussi
     */
    bool set(const std::string& key, const std::string& value);

    /**
     * @brief Récupère une valeur entière
     * @param key Nom de la constante
     * @return Valeur ou std::nullopt si inexistante
     */
    std::optional<int> getInt(const std::string& key) const;

    /**
     * @brief Récupère une valeur booléenne
     * @param key Nom de la constante
     * @return Valeur ou std::nullopt si inexistante
     */
    std::optional<bool> getBool(const std::string& key) const;

    /**
     * @brief Liste toutes les configurations disponibles
     * @return Map des configurations avec leurs valeurs actuelles
     */
    std::unordered_map<std::string, std::string> listAll() const;

    /**
     * @brief Réinitialise toutes les valeurs aux valeurs par défaut
     */
    void reset();

private:
    RuntimeConfig();

    void initializeDefinitions();
    bool validateValue(const std::string& key, const std::string& value, const ConfigDef& def);

    mutable std::mutex configMutex;
    std::unordered_map<std::string, std::string> config;
    std::unordered_map<std::string, ConfigDef> definitions;
};

#endif
