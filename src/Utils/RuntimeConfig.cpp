#include "Utils/RuntimeConfig.hpp"
#include "Utils/Constants.hpp"
#include "Utils/Logger.hpp"

RuntimeConfig::RuntimeConfig() {
    initializeDefinitions();
    for (const auto& [key, def] : definitions) {
        config[key] = def.defaultValue;
    }
}

void RuntimeConfig::initializeDefinitions() {
    using namespace Constants;
    
    // Format: { type, defaultValue, min, max }
    definitions["HEARTBEAT_INTERVAL_S"]    = { ConfigType::INT, std::to_string(HEARTBEAT_INTERVAL_S), MIN_HEARTBEAT_INTERVAL_S, 3600 };
    definitions["HEARTBEAT_CHECK_DELAY_S"] = { ConfigType::INT, std::to_string(HEARTBEAT_CHECK_DELAY_S), 1, 60 };
    definitions["HEARTBEAT_TIMEOUT_S"]     = { ConfigType::INT, std::to_string(HEARTBEAT_TIMEOUT_S), MIN_HEARTBEAT_TIMEOUT_S, 3600 };
    definitions["CLIENT_TIMEOUT_S"]        = { ConfigType::INT, std::to_string(CLIENT_TIMEOUT_S), 10, 3600 };
    definitions["MAX_QUEUE_SIZE"]          = { ConfigType::INT, std::to_string(MAX_QUEUE_SIZE), 10, 100000 };
    definitions["THREAD_POOL_SIZE"]        = { ConfigType::INT, std::to_string(THREAD_POOL_SIZE), 1, 128 };
    definitions["MAX_USERNAME_LENGTH"]     = { ConfigType::INT, std::to_string(MAX_USERNAME_LENGTH), MIN_USERNAME_LENGTH, MAX_USERNAME_LENGTH_LIMIT };
    definitions["MAX_SUBJECT_LENGTH"]      = { ConfigType::INT, std::to_string(MAX_SUBJECT_LENGTH), MIN_SUBJECT_LENGTH, MAX_SUBJECT_LENGTH_LIMIT };
    definitions["AUTO_STOP_WHEN_NO_CLIENTS"] = { ConfigType::BOOL, AUTO_STOP_WHEN_NO_CLIENTS ? "true" : "false", 0, 0 };
}

bool RuntimeConfig::validateValue(const std::string& key, const std::string& value, const ConfigDef& def) {
    if (def.type == ConfigType::BOOL) {
        if (value != "true" && value != "false" && value != "1" && value != "0") {
            LOG_WARNING("Invalid value for " + key + " (expected: true/false/1/0)");
            return false;
        }
        return true;
    }
    
    // ConfigType::INT
    try {
        int numValue = std::stoi(value);
        if (numValue < def.minValue || numValue > def.maxValue) {
            LOG_WARNING(key + " must be between " + std::to_string(def.minValue) + 
                       " and " + std::to_string(def.maxValue));
            return false;
        }
        return true;
    } catch (...) {
        LOG_WARNING("Invalid value for " + key + " (expected: integer)");
        return false;
    }
}

bool RuntimeConfig::set(const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(configMutex);
    
    auto it = definitions.find(key);
    if (it == definitions.end()) {
        LOG_WARNING("Unknown configuration: " + key);
        return false;
    }
    
    if (!validateValue(key, value, it->second)) {
        return false;
    }
    
    config[key] = value;
    LOG_INFO("Configuration modified: " + key + " = " + value);
    return true;
}

std::optional<int> RuntimeConfig::getInt(const std::string& key) const {
    std::lock_guard<std::mutex> lock(configMutex);
    auto it = config.find(key);
    if (it == config.end()) {
        return std::nullopt;
    }
    try {
        return std::stoi(it->second);
    } catch (...) {
        return std::nullopt;
    }
}

std::optional<bool> RuntimeConfig::getBool(const std::string& key) const {
    std::lock_guard<std::mutex> lock(configMutex);
    auto it = config.find(key);
    if (it == config.end()) {
        return std::nullopt;
    }
    return (it->second == "true" || it->second == "1");
}

std::unordered_map<std::string, std::string> RuntimeConfig::listAll() const {
    std::lock_guard<std::mutex> lock(configMutex);
    return config;
}

void RuntimeConfig::reset() {
    std::lock_guard<std::mutex> lock(configMutex);
    for (const auto& [key, def] : definitions) {
        config[key] = def.defaultValue;
    }
    LOG_INFO("Configurations reset to default values");
}
