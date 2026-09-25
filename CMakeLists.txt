cmake_minimum_required(VERSION 3.20)

project(SanbuCraftAI VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(SANBUCRAFT_BUILD_TESTS "Build SanbuCraft AI tests" ON)
option(SANBUCRAFT_BUILD_GUI "Build the Qt desktop dashboard when Qt 6 is available" ON)

find_package(ZLIB REQUIRED)
find_package(SQLite3 REQUIRED)

add_library(sanbucraft_core
    src/core/Config.cpp
    src/core/Logger.cpp
    src/minecraft/Nbt.cpp
    src/minecraft/WorldLoader.cpp
    src/minecraft/PlayerLoader.cpp
    src/minecraft/ContainerScanner.cpp
    src/database/Database.cpp
    src/analysis/WorldAnalyzer.cpp
)

target_include_directories(sanbucraft_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_features(sanbucraft_core PUBLIC cxx_std_17)
target_link_libraries(sanbucraft_core PUBLIC ZLIB::ZLIB SQLite::SQLite3)

if(MSVC)
    target_compile_options(sanbucraft_core PRIVATE /W4 /permissive-)
else()
    target_compile_options(sanbucraft_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(sanbucraft_ai src/main.cpp)
target_link_libraries(sanbucraft_ai PRIVATE sanbucraft_core)

if(SANBUCRAFT_BUILD_GUI)
    find_package(Qt6 COMPONENTS Widgets QUIET)
    if(Qt6Widgets_FOUND)
        qt_standard_project_setup()
        add_library(sanbucraft_gui
            src/gui/MainWindow.cpp
            src/gui/MainWindow.h
            src/gui/MapView.cpp
            src/gui/MapView.h
        )
        target_link_libraries(sanbucraft_gui PUBLIC sanbucraft_core Qt6::Widgets)
        target_include_directories(sanbucraft_gui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
        target_compile_definitions(sanbucraft_ai PRIVATE SANBUCRAFT_WITH_QT_GUI=1)
        target_link_libraries(sanbucraft_ai PRIVATE sanbucraft_gui)
    else()
        message(STATUS "Qt 6 Widgets not found: building the headless executable only")
    endif()
endif()

include(CTest)
if(BUILD_TESTING AND SANBUCRAFT_BUILD_TESTS)
    add_executable(config_tests tests/ConfigTests.cpp)
    target_link_libraries(config_tests PRIVATE sanbucraft_core)
    add_test(NAME config_tests COMMAND config_tests)

    add_executable(logger_tests tests/LoggerTests.cpp)
    target_link_libraries(logger_tests PRIVATE sanbucraft_core)
    add_test(NAME logger_tests COMMAND logger_tests)

    add_executable(nbt_tests tests/NbtTests.cpp)
    target_link_libraries(nbt_tests PRIVATE sanbucraft_core)
    add_test(NAME nbt_tests COMMAND nbt_tests)

    add_executable(analysis_tests tests/AnalysisTests.cpp)
    target_link_libraries(analysis_tests PRIVATE sanbucraft_core)
    add_test(NAME analysis_tests COMMAND analysis_tests)
endif()
