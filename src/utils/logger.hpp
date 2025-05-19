// File: logger.hpp
// Path: src/utils/logger.hpp
#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>
#include <fstream>
#include <mutex>
#include <ctime>
#include <sstream>

class Logger {
public:
    Logger(const std::string& filename);
    ~Logger();

    void logError(const std::string& message);
    void logInfo(const std::string& message);
    void logWarning(const std::string& message);

private:
    std::ofstream file_;
    std::mutex mutex_;
    std::string getCurrentTime() const;
    void log(const std::string& level, const std::string& message);
};

#endif // LOGGER_HPP