#include "core/Config.h"

#include <charconv>
#include <fstream>
#include <map>
#include <sstream>

namespace sanbucraft::core {
namespace {

std::string trim(const std::string& value) {
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    const auto last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

bool parsePort(const std::string& text, unsigned short& port) {
    unsigned int parsed = 0;
    const auto result = std::from_chars(text.data(), text.data() + text.size(), parsed);
    if (result.ec != std::errc{} || result.ptr != text.data() + text.size() || parsed == 0 || parsed > 65535) return false;
    port = static_cast<unsigned short>(parsed);
    return true;
}

}  // namespace

bool Config::load(const std::filesystem::path& path, AppConfig& config, std::string& error) {
    std::ifstream input(path);
    if (!input.is_open()) {
        error = "Cannot open configuration file: " + path.string();
        return false;
    }

    std::string line;
    std::size_t lineNumber = 0;
    while (std::getline(input, line)) {
        ++lineNumber;
        line = trim(line);
        if (line.empty() || line.front() == '#') continue;
        const auto separator = line.find('=');
        if (separator == std::string::npos) {
            error = "Invalid configuration at line " + std::to_string(lineNumber) + ": expected key=value.";
            return false;
        }
        const std::string key = trim(line.substr(0, separator));
        const std::string value = trim(line.substr(separator + 1));
        if (key == "minecraft_world_path") config.minecraftWorldPath = value;
        else if (key == "database_path") config.databasePath = value;
        else if (key == "log_path") config.logPath = value;
        else if (key == "ai_provider") config.aiProvider = value;
        else if (key == "ai_model") config.aiModel = value;
        else if (key == "server_address") config.serverAddress = value;
        else if (key == "rcon_port" && !parsePort(value, config.rconPort)) {
            error = "Invalid rcon_port at line " + std::to_string(lineNumber) + ".";
            return false;
        } else if (key != "rcon_port") {
            error = "Unknown configuration key at line " + std::to_string(lineNumber) + ": " + key;
            return false;
        }
    }
    return validate(config, error);
}

bool Config::save(const std::filesystem::path& path, const AppConfig& config, std::string& error) {
    if (!validate(config, error)) return false;
    std::error_code filesystemError;
    if (!path.parent_path().empty()) std::filesystem::create_directories(path.parent_path(), filesystemError);
    if (filesystemError) {
        error = "Cannot create configuration directory: " + filesystemError.message();
        return false;
    }
    std::ofstream output(path, std::ios::trunc);
    if (!output.is_open()) {
        error = "Cannot write configuration file: " + path.string();
        return false;
    }
    output << "# SanbuCraft AI local configuration. Do not place API keys or RCON passwords here.\n"
           << "minecraft_world_path=" << config.minecraftWorldPath.string() << '\n'
           << "database_path=" << config.databasePath.string() << '\n'
           << "log_path=" << config.logPath.string() << '\n'
           << "ai_provider=" << config.aiProvider << '\n'
           << "ai_model=" << config.aiModel << '\n'
           << "server_address=" << config.serverAddress << '\n'
           << "rcon_port=" << config.rconPort << '\n';
    if (!output.good()) {
        error = "Failed while writing configuration file: " + path.string();
        return false;
    }
    return true;
}

bool Config::validate(const AppConfig& config, std::string& error) {
    if (config.aiProvider.empty() || config.aiProvider.size() > 64) {
        error = "ai_provider must contain 1 to 64 characters.";
        return false;
    }
    if (config.serverAddress.empty() || config.serverAddress.size() > 255) {
        error = "server_address must contain 1 to 255 characters.";
        return false;
    }
    if (config.rconPort == 0) {
        error = "rcon_port must be between 1 and 65535.";
        return false;
    }
    return true;
}

}  // namespace sanbucraft::core
