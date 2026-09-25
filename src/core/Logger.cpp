#include "core/Logger.h"

#include <chrono>
#include <ctime>
#include <iostream>
#include <sstream>

namespace sanbucraft::core {

Logger::Logger(const LogLevel minimumLevel) : minimumLevel_(minimumLevel) {}

bool Logger::setLogFile(const std::filesystem::path& path, std::string& error) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::error_code filesystemError;
    if (!path.parent_path().empty()) {
        std::filesystem::create_directories(path.parent_path(), filesystemError);
    }
    if (filesystemError) {
        error = "Cannot create log directory: " + filesystemError.message();
        return false;
    }

    file_.close();
    file_.open(path, std::ios::out | std::ios::app);
    if (!file_.is_open()) {
        error = "Cannot open log file: " + path.string();
        return false;
    }
    return true;
}

void Logger::setMinimumLevel(const LogLevel level) { minimumLevel_ = level; }

void Logger::log(const LogLevel level, const std::string& message) {
    if (static_cast<int>(level) < static_cast<int>(minimumLevel_)) {
        return;
    }

    const std::string line = "[" + timestamp() + "] [" + toString(level) + "] " + message;
    std::lock_guard<std::mutex> lock(mutex_);
    std::clog << line << '\n';
    if (file_.is_open()) {
        file_ << line << '\n';
        file_.flush();
    }
}

void Logger::debug(const std::string& message) { log(LogLevel::Debug, message); }
void Logger::info(const std::string& message) { log(LogLevel::Info, message); }
void Logger::warning(const std::string& message) { log(LogLevel::Warning, message); }
void Logger::error(const std::string& message) { log(LogLevel::Error, message); }

const char* Logger::toString(const LogLevel level) {
    switch (level) {
        case LogLevel::Debug: return "DEBUG";
        case LogLevel::Info: return "INFO";
        case LogLevel::Warning: return "WARNING";
        case LogLevel::Error: return "ERROR";
    }
    return "UNKNOWN";
}

std::string Logger::timestamp() {
    const auto now = std::chrono::system_clock::now();
    const std::time_t currentTime = std::chrono::system_clock::to_time_t(now);
    std::tm localTime{};
#ifdef _WIN32
    localtime_s(&localTime, &currentTime);
#else
    localtime_r(&currentTime, &localTime);
#endif
    std::ostringstream output;
    output << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S");
    return output.str();
}

}  // namespace sanbucraft::core
