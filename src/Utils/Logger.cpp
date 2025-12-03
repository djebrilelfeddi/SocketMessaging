#include "Utils/Logger.hpp"
#include <iostream>

void Logger::setLogFile(const std::string& filename) {
    std::lock_guard<std::mutex> lock(logMutex);
    
    if (logFile.is_open()) {
        logFile.close();
    }
    
    logFile.open(filename, std::ios::app);
    if (!logFile.is_open()) {
        std::cerr << "Error: Cannot open log file: " << filename << "\n";
    }
}

void Logger::setVerbose(bool enabled) {
    std::lock_guard<std::mutex> lock(logMutex);
    verbose = enabled;
}

void Logger::log(LogLevel level, const std::string& message) {
    std::lock_guard<std::mutex> lock(logMutex);
    
    std::string timestamp = getCurrentTimestamp();
    std::string levelStr = levelToString(level);
    
    std::string logEntry = "[" + timestamp + "] [" + levelStr + "] " + message;
    
    if (logFile.is_open()) {
        logFile << logEntry << std::endl;
    }
    
    // Console display: INFO and above, or DEBUG if verbose enabled
    if (level != LogLevel::DEBUG || verbose) {
        std::string color = getColorForLevel(level);
        std::cout << color << "[" << timestamp << "] [" << levelStr << "]" << Color::RESET << " " << message << std::endl;
    }
}

std::string Logger::getColorForLevel(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG:      return Color::GRAY;
        case LogLevel::INFO:       return Color::BRIGHT_CYAN;
        case LogLevel::WARNING:    return Color::BRIGHT_YELLOW;
        case LogLevel::ERROR:      return Color::BRIGHT_RED;
        case LogLevel::CONNECT:    return Color::BRIGHT_GREEN;
        case LogLevel::DISCONNECT: return Color::BRIGHT_MAGENTA;
        default:                   return Color::RESET;
    }
}

std::string Logger::levelToString(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO: return "INFO";
        case LogLevel::WARNING: return "WARNING";
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::CONNECT: return "CONNECT";
        case LogLevel::DISCONNECT: return "DISCONNECT";
        default: return "UNKNOWN";
    }
}

std::string Logger::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()
    ) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    
    return ss.str();
}