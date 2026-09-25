#include "core/Config.h"
#include "core/Logger.h"

#include <filesystem>
#include <iostream>

namespace {

void printUsage() {
    std::cout << "SanbuCraft AI (Phase 1)\n"
              << "Usage: sanbucraft_ai [--config <path>] [--world <minecraft-world-path>]\n"
              << "                     [--log-level debug|info|warning|error] [--help]\n";
}

bool parseLogLevel(const std::string& text, sanbucraft::core::LogLevel& level) {
    using sanbucraft::core::LogLevel;
    if (text == "debug") level = LogLevel::Debug;
    else if (text == "info") level = LogLevel::Info;
    else if (text == "warning") level = LogLevel::Warning;
    else if (text == "error") level = LogLevel::Error;
    else return false;
    return true;
}

}  // namespace

int main(int argc, char* argv[]) {
    namespace fs = std::filesystem;
    using namespace sanbucraft::core;

    fs::path configPath = "data/sanbucraft.conf";
    fs::path suppliedWorldPath;
    LogLevel logLevel = LogLevel::Info;

    for (int index = 1; index < argc; ++index) {
        const std::string argument = argv[index];
        if (argument == "--help") { printUsage(); return 0; }
        if ((argument == "--config" || argument == "--world" || argument == "--log-level") && index + 1 >= argc) {
            std::cerr << "Missing value for " << argument << ".\n";
            return 2;
        }
        if (argument == "--config") configPath = argv[++index];
        else if (argument == "--world") suppliedWorldPath = argv[++index];
        else if (argument == "--log-level" && !parseLogLevel(argv[++index], logLevel)) {
            std::cerr << "Invalid log level. Use debug, info, warning, or error.\n";
            return 2;
        } else if (argument != "--log-level") {
            std::cerr << "Unknown argument: " << argument << "\n";
            printUsage();
            return 2;
        }
    }

    AppConfig config;
    std::string error;
    if (fs::exists(configPath)) {
        if (!Config::load(configPath, config, error)) {
            std::cerr << "Configuration error: " << error << "\n";
            return 1;
        }
    } else if (!Config::save(configPath, config, error)) {
        std::cerr << "Configuration error: " << error << "\n";
        return 1;
    }
    if (!suppliedWorldPath.empty()) config.minecraftWorldPath = suppliedWorldPath;

    Logger logger(logLevel);
    if (!logger.setLogFile(config.logPath, error)) {
        std::cerr << "Logger warning: " << error << "\n";
    }
    logger.info("Starting SanbuCraft AI Phase 1.");

    std::cout << "========================================\n"
              << "          SanbuCraft AI | Phase 1\n"
              << "========================================\n"
              << "Configuration: " << configPath << "\n"
              << "World path: " << (config.minecraftWorldPath.empty() ? "not selected" : config.minecraftWorldPath.string()) << "\n"
              << "AI provider: " << config.aiProvider << "\n"
              << "Status: Project skeleton, configuration, and logger are ready.\n";
    logger.info("Phase 1 initialization completed.");
    return 0;
}
