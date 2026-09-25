#include "core/Config.h"

#include <filesystem>
#include <fstream>
#include <iostream>

int main() {
    namespace fs = std::filesystem;
    const fs::path directory = fs::temp_directory_path() / "sanbucraft_config_tests";
    const fs::path file = directory / "config.conf";
    std::error_code ignored;
    fs::remove_all(directory, ignored);

    sanbucraft::core::AppConfig original;
    original.minecraftWorldPath = "/tmp/Survival_01";
    original.aiProvider = "mock";
    original.rconPort = 25565;
    std::string error;
    if (!sanbucraft::core::Config::save(file, original, error)) {
        std::cerr << error << '\n'; return 1;
    }
    sanbucraft::core::AppConfig loaded;
    if (!sanbucraft::core::Config::load(file, loaded, error) || loaded.minecraftWorldPath != original.minecraftWorldPath || loaded.rconPort != 25565) {
        std::cerr << "Round-trip config test failed: " << error << '\n'; return 1;
    }
    std::ofstream invalid(file, std::ios::trunc);
    invalid << "unknown_key=value\n";
    invalid.close();
    if (sanbucraft::core::Config::load(file, loaded, error)) {
        std::cerr << "Unknown-key config test failed.\n"; return 1;
    }
    fs::remove_all(directory, ignored);
    return 0;
}
