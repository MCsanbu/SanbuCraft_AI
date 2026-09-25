#include "core/Logger.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

int main() {
    const auto file = std::filesystem::temp_directory_path() / "sanbucraft_logger_test.log";
    std::error_code ignored;
    std::filesystem::remove(file, ignored);
    sanbucraft::core::Logger logger(sanbucraft::core::LogLevel::Info);
    std::string error;
    if (!logger.setLogFile(file, error)) { std::cerr << error << '\n'; return 1; }
    logger.debug("must not appear");
    logger.info("logger test entry");
    std::ifstream input(file);
    const std::string contents((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
    std::filesystem::remove(file, ignored);
    if (contents.find("[INFO] logger test entry") == std::string::npos || contents.find("must not appear") != std::string::npos) {
        std::cerr << "Unexpected logger output.\n"; return 1;
    }
    return 0;
}
