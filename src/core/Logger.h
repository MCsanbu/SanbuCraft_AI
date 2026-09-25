#pragma once

#include <filesystem>
#include <fstream>
#include <mutex>
#include <string>

namespace sanbucraft::core {

enum class LogLevel { Debug, Info, Warning, Error };

class Logger {
public:
    explicit Logger(LogLevel minimumLevel = LogLevel::Info);

    bool setLogFile(const std::filesystem::path& path, std::string& error);
    void setMinimumLevel(LogLevel level);
    void log(LogLevel level, const std::string& message);

    void debug(const std::string& message);
    void info(const std::string& message);
    void warning(const std::string& message);
    void error(const std::string& message);

    static const char* toString(LogLevel level);

private:
    static std::string timestamp();

    LogLevel minimumLevel_;
    std::ofstream file_;
    std::mutex mutex_;
};

}  // namespace sanbucraft::core
