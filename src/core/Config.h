#pragma once

#include <filesystem>
#include <string>

namespace sanbucraft::core {

struct AppConfig {
    std::filesystem::path minecraftWorldPath;
    std::filesystem::path databasePath = "data/sanbucraft.db";
    std::filesystem::path logPath = "data/sanbucraft.log";
    std::string aiProvider = "mock";
    std::string aiModel;
    std::string serverAddress = "127.0.0.1";
    unsigned short rconPort = 25575;
};

class Config {
public:
    static bool load(const std::filesystem::path& path, AppConfig& config, std::string& error);
    static bool save(const std::filesystem::path& path, const AppConfig& config, std::string& error);
    static bool validate(const AppConfig& config, std::string& error);
};

}  // namespace sanbucraft::core
