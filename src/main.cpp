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
#include <memory>
#include <vector>

#ifdef SANBUCRAFT_WITH_QT_GUI
#include "gui/MainWindow.h"

#include <QApplication>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QString>
#endif

namespace {

void printUsage() {
    std::cout << "SanbuCraft AI\n"
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

#ifdef SANBUCRAFT_WITH_QT_GUI

QString minecraftSavesDirectory() {
#ifdef Q_OS_WIN
    const QString appData = qEnvironmentVariable("APPDATA");
    if (!appData.isEmpty()) {
        return QDir(appData).filePath(".minecraft/saves");
    }
    return QDir::home().filePath("AppData/Roaming/.minecraft/saves");
#elif defined(Q_OS_MACOS)
    return QDir::home().filePath("Library/Application Support/minecraft/saves");
#else
    return QDir::home().filePath(".minecraft/saves");
#endif
}

bool selectWorldFolder(std::filesystem::path& worldPath) {
    const QString savesDirectory = minecraftSavesDirectory();
    const QString startDirectory =
        QDir(savesDirectory).exists() ? savesDirectory : QDir::homePath();

    const QString selected = QFileDialog::getExistingDirectory(
        nullptr,
        QStringLiteral("选择 Minecraft Java 存档文件夹"),
        startDirectory,
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (selected.isEmpty()) {
        return false;
    }

    const QDir directory(selected);
    if (!directory.exists("level.dat")) {
        QMessageBox::critical(
            nullptr,
            QStringLiteral("不是有效的 Minecraft 存档"),
            QStringLiteral("你选择的文件夹中没有找到 level.dat。\n\n"
                           "请选择 .minecraft/saves/ 下的具体世界文件夹。"));
        return false;
    }

    worldPath = std::filesystem::path(selected.toStdWString());
    return true;
}

#endif

}  // namespace

int main(int argc, char* argv[]) {
    namespace fs = std::filesystem;
    using namespace sanbucraft::core;

    fs::path configPath = "data/sanbucraft.conf";
    fs::path suppliedWorldPath;
    LogLevel logLevel = LogLevel::Info;
    bool headless = false;
    bool configWasSupplied = false;
    std::string question;

    for (int index = 1; index < argc; ++index) {
        const std::string argument = argv[index];
        if (argument == "--help") {
            printUsage();
            return 0;
        }
        if ((argument == "--config" || argument == "--world" || argument == "--ask" || argument == "--log-level") &&
            index + 1 >= argc) {
            std::cerr << "Missing value for " << argument << ".\n";
            return 2;
        }

        if (argument == "--config") {
            configPath = argv[++index];
            configWasSupplied = true;
        } else if (argument == "--world") {
            suppliedWorldPath = argv[++index];
        } else if (argument == "--ask") {
            question = argv[++index];
        } else if (argument == "--headless") {
            headless = true;
        } else if (argument == "--log-level") {
            if (!parseLogLevel(argv[++index], logLevel)) {
                std::cerr << "Invalid log level. Use debug, info, warning, or error.\n";
                return 2;
            }
        } else {
            std::cerr << "Unknown argument: " << argument << "\n";
            printUsage();
            return 2;
        }
    }

#ifdef SANBUCRAFT_WITH_QT_GUI
    std::unique_ptr<QApplication> application;
    if (!headless && question.empty()) {
        application = std::make_unique<QApplication>(argc, argv);
        if (!configWasSupplied) {
            configPath =
                fs::path(QCoreApplication::applicationDirPath().toStdWString()) /
                "data" / "sanbucraft.conf";
        }
    }
#endif

    AppConfig config;
    std::string error;
    if (fs::exists(configPath)) {
        if (!Config::load(configPath, config, error)) {
#ifdef SANBUCRAFT_WITH_QT_GUI
            if (application) {
                QMessageBox::critical(nullptr, QStringLiteral("配置错误"),
                                      QString::fromStdString(error));
            }
#endif
            std::cerr << "Configuration error: " << error << "\n";
            return 1;
        }
    } else if (!Config::save(configPath, config, error)) {
#ifdef SANBUCRAFT_WITH_QT_GUI
        if (application) {
            QMessageBox::critical(nullptr, QStringLiteral("配置错误"),
                                  QString::fromStdString(error));
        }
#endif
        std::cerr << "Configuration error: " << error << "\n";
        return 1;
    }

#ifdef SANBUCRAFT_WITH_QT_GUI
    if (application && suppliedWorldPath.empty()) {
        if (!selectWorldFolder(suppliedWorldPath)) {
            return 0;
        }

        config.minecraftWorldPath = suppliedWorldPath;
        if (!Config::save(configPath, config, error)) {
            QMessageBox::warning(
                nullptr,
                QStringLiteral("保存配置失败"),
                QString::fromStdString(error));
        }
    }
#endif

    if (!suppliedWorldPath.empty()) {
        config.minecraftWorldPath = suppliedWorldPath;
    }

    Logger logger(logLevel);
    if (!logger.setLogFile(config.logPath, error)) {
#ifdef SANBUCRAFT_WITH_QT_GUI
        if (application) {
            QMessageBox::warning(nullptr, QStringLiteral("日志提示"),
                                 QString::fromStdString(error));
        }
#endif
        std::cerr << "Logger warning: " << error << "\n";
    }

    logger.info("Starting SanbuCraft AI.");

    if (!config.minecraftWorldPath.empty()) {
        sanbucraft::minecraft::WorldInfo world;
        std::vector<sanbucraft::minecraft::Player> players;
        std::vector<sanbucraft::minecraft::Container> containers;

        if (!sanbucraft::minecraft::WorldLoader::loadWorld(
                config.minecraftWorldPath.string(), world, error) ||
            !sanbucraft::minecraft::PlayerLoader::loadPlayers(
                config.minecraftWorldPath.string(), players, error) ||
            !sanbucraft::minecraft::ContainerScanner::scan(
                config.minecraftWorldPath.string(), containers, error)) {
#ifdef SANBUCRAFT_WITH_QT_GUI
            if (application) {
                QMessageBox::critical(nullptr, QStringLiteral("世界读取失败"),
                                      QString::fromStdString(error));
            }
#endif
            logger.error(error);
            std::cerr << "World analysis error: " << error << "\n";
            return 1;
        }

        const auto analysis =
            sanbucraft::analysis::WorldAnalyzer::analyze(players, containers);

        sanbucraft::database::Database database;
        if (!database.open(config.databasePath.string(), error) ||
            !database.replaceWorldData(world, players, containers, error)) {
#ifdef SANBUCRAFT_WITH_QT_GUI
            if (application) {
                QMessageBox::critical(nullptr, QStringLiteral("数据库错误"),
                                      QString::fromStdString(error));
            }
#endif
            logger.error(error);
            std::cerr << "Database error: " << error << "\n";
            return 1;
        }

        logger.info("World analyzed: " + world.name +
                    ", players=" + std::to_string(players.size()) +
                    ", containers=" + std::to_string(analysis.containers));

        if (!question.empty()) {
            sanbucraft::ai::MockAIProvider provider;
            sanbucraft::ai::Agent agent(provider);
            const auto reply = agent.ask(
                sanbucraft::ai::buildWorldContext(world, players, containers),
                question);

            std::cout << "AI Assistant (" << provider.name() << "):\n"
                      << reply.answer << "\n";
            if (reply.requiresConfirmation) {
                std::cout << "Confirmation required: "
                          << reply.proposedAction << "\n";
            }
            return 0;
        }

#ifdef SANBUCRAFT_WITH_QT_GUI
        if (application) {
            sanbucraft::gui::MainWindow window(world, players, containers);
            window.show();
            return application->exec();
        }
#endif
    }

    std::cout << "========================================\n"
              << "              SanbuCraft AI\n"
              << "========================================\n"
              << "Configuration: " << configPath << "\n"
              << "World path: "
              << (config.minecraftWorldPath.empty()
                      ? "not selected"
                      : config.minecraftWorldPath.string())
              << "\n"
              << "AI provider: " << config.aiProvider << "\n"
              << "Status: ready.\n";

    logger.info("Initialization completed.");
    return 0;
}
