#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>
#include <fstream>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <sstream>
#include "Colors.hpp"

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    CONNECT,
    DISCONNECT
};

class Logger {
public:
    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }

    void setLogFile(const std::string& filename);
    void setVerbose(bool enabled);
    void log(LogLevel level, const std::string& message);

private:
    Logger() = default;
    ~Logger() {
        if (logFile.is_open()) {
            logFile.close();
        }
    }

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    std::string levelToString(LogLevel level);
    std::string getColorForLevel(LogLevel level);
    std::string getCurrentTimestamp();

    std::ofstream logFile;
    std::mutex logMutex;
    bool verbose = false;
};

// Macros pour simplifier l'utilisation
#define LOG_DEBUG(msg) Logger::getInstance().log(LogLevel::DEBUG, msg)
#define LOG_INFO(msg) Logger::getInstance().log(LogLevel::INFO, msg)
#define LOG_WARNING(msg) Logger::getInstance().log(LogLevel::WARNING, msg)
#define LOG_ERROR(msg) Logger::getInstance().log(LogLevel::ERROR, msg)
#define LOG_CONNECT(msg) Logger::getInstance().log(LogLevel::CONNECT, msg)
#define LOG_DISCONNECT(msg) Logger::getInstance().log(LogLevel::DISCONNECT, msg)

#endif // LOGGER_HPP
