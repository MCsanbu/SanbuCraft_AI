#include "core/Config.h"
#include "core/Logger.h"
#include "minecraft/WorldLoader.h"
#include "minecraft/PlayerLoader.h"
#include "minecraft/ContainerScanner.h"
#include "analysis/WorldAnalyzer.h"
#include "database/Database.h"
#include "ai/Agent.h"
#include "ai/MockAIProvider.h"

#include <filesystem>
#include <iostream>
#include <vector>

#ifdef SANBUCRAFT_WITH_QT_GUI
#include "gui/MainWindow.h"
#include <QApplication>
#endif

namespace {

void printUsage() {
    std::cout << "SanbuCraft AI (Phase 1)\n"
              << "Usage: sanbucraft_ai [--config <path>] [--world <minecraft-world-path>]\n"
              << "                     [--ask <question>] [--log-level debug|info|warning|error] [--headless] [--help]\n";
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
    bool headless = false;
    std::string question;

    for (int index = 1; index < argc; ++index) {
        const std::string argument = argv[index];
        if (argument == "--help") { printUsage(); return 0; }
        if ((argument == "--config" || argument == "--world" || argument == "--ask" || argument == "--log-level") && index + 1 >= argc) {
            std::cerr << "Missing value for " << argument << ".\n";
            return 2;
        }
        if (argument == "--config") configPath = argv[++index];
        else if (argument == "--world") suppliedWorldPath = argv[++index];
        else if (argument == "--ask") question = argv[++index];
        else if (argument == "--headless") headless = true;
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

    if (!config.minecraftWorldPath.empty()) {
        sanbucraft::minecraft::WorldInfo world;
        std::vector<sanbucraft::minecraft::Player> players;
        std::vector<sanbucraft::minecraft::Container> containers;
        if (!sanbucraft::minecraft::WorldLoader::loadWorld(config.minecraftWorldPath.string(), world, error) ||
            !sanbucraft::minecraft::PlayerLoader::loadPlayers(config.minecraftWorldPath.string(), players, error) ||
            !sanbucraft::minecraft::ContainerScanner::scan(config.minecraftWorldPath.string(), containers, error)) {
            logger.error(error);
            std::cerr << "World analysis error: " << error << "\n";
            return 1;
        }
        const auto analysis = sanbucraft::analysis::WorldAnalyzer::analyze(players, containers);
        sanbucraft::database::Database database;
        if (!database.open(config.databasePath.string(), error) || !database.replaceWorldData(world, players, containers, error)) {
            logger.error(error);
            std::cerr << "Database error: " << error << "\n";
            return 1;
        }
        logger.info("World analyzed: " + world.name + ", players=" + std::to_string(players.size()) + ", containers=" + std::to_string(analysis.containers));
        if (!question.empty()) {
            sanbucraft::ai::MockAIProvider provider;
            sanbucraft::ai::Agent agent(provider);
            const auto reply = agent.ask(sanbucraft::ai::buildWorldContext(world, players, containers), question);
            std::cout << "AI Assistant (" << provider.name() << "):\n" << reply.answer << "\n";
            if (reply.requiresConfirmation) std::cout << "Confirmation required: " << reply.proposedAction << "\n";
            return 0;
        }
#ifdef SANBUCRAFT_WITH_QT_GUI
        if (!headless) {
            QApplication application(argc, argv);
            sanbucraft::gui::MainWindow window(world, players, containers);
            window.show();
            return application.exec();
        }
#else
        (void)headless;
#endif
    }

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
