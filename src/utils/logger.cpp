// File: logger.cpp
// Path: src/utils/logger.cpp
#include "logger.hpp"
#include <iomanip>

Logger::Logger(const std::string& filename) {
    file_.open(filename, std::ios::app);
    if (!file_.is_open()) {
        throw std::runtime_error("Unable to open log file: " + filename);
    }
}

Logger::~Logger() {
    if (file_.is_open()) {
        file_.close();
    }
}

std::string Logger::getCurrentTime() const {
    auto now = std::time(nullptr);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&now), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

void Logger::log(const std::string& level, const std::string& message) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (file_.is_open()) {
        file_ << "[" << getCurrentTime() << "] [" << level << "] " << message << std::endl;
    }
}

void Logger::logError(const std::string& message) {
    log("ERROR", message);
}

void Logger::logInfo(const std::string& message) {
    log("INFO", message);
}

void Logger::logWarning(const std::string& message) {
    log("WARNING", message);
}