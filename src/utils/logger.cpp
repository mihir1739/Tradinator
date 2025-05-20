// File: logger.cpp
// Path: src/utils/logger.cpp
#include "logger.hpp"
#include <iomanip>
/**
 * @brief Logger constructor
 */
Logger::Logger(const std::string& filename) {
    file_.open(filename, std::ios::app);
    if (!file_.is_open()) {
        throw std::runtime_error("Unable to open log file: " + filename);
    }
}

/**
 * @brief Logger destructor
 */
Logger::~Logger() {
    if (file_.is_open()) {
        file_.close();
    }
}

/**
 * @brief Gets the current time as a formatted string
 * 
 * @return Formatted current time string
 */
std::string Logger::getCurrentTime() const {
    auto now = std::time(nullptr);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&now), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

/**
 * @brief Logs a message with a specific level
 * 
 * @param level The log level (e.g., ERROR, INFO, WARNING)
 * @param message The message to log
 */
void Logger::log(const std::string& level, const std::string& message) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (file_.is_open()) {
        file_ << "[" << getCurrentTime() << "] [" << level << "] " << message << std::endl;
    }
}

/**
 * @brief Logs an error message
 * 
 * @param message The error message to log
 */
void Logger::logError(const std::string& message) {
    log("ERROR", message);
}

/**
 * @brief Logs an info message
 * 
 * @param message The info message to log
 */
void Logger::logInfo(const std::string& message) {
    log("INFO", message);
}

/**
 * @brief Logs a warning message
 * 
 * @param message The warning message to log
 */
void Logger::logWarning(const std::string& message) {
    log("WARNING", message);
}