/**
 * @file RuntimeConfig.hpp
 * @brief Runtime configuration modifiable via commands
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
 * @brief Configuration value type
 */
enum class ConfigType { INT, BOOL };

/**
 * @struct ConfigDef
 * @brief Configuration definition with its constraints
 */
struct ConfigDef {
    ConfigType type;
    std::string defaultValue;
    int minValue = 0;      // For INT only
    int maxValue = INT_MAX; // For INT only
};

/**
 * @class RuntimeConfig
 * @brief Singleton to manage runtime modifiable configurations
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
     * @brief Sets a configuration value
     * @param key Constant name
     * @param value New value (string)
     * @return true if modification succeeded
     */
    bool set(const std::string& key, const std::string& value);

    /**
     * @brief Gets an integer value
     * @param key Constant name
     * @return Value or std::nullopt if not found
     */
    std::optional<int> getInt(const std::string& key) const;

    /**
     * @brief Gets a boolean value
     * @param key Constant name
     * @return Value or std::nullopt if not found
     */
    std::optional<bool> getBool(const std::string& key) const;

    /**
     * @brief Lists all available configurations
     * @return Map of configurations with their current values
     */
    std::unordered_map<std::string, std::string> listAll() const;

    /**
     * @brief Resets all values to defaults
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
