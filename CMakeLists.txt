cmake_minimum_required(VERSION 3.20)

project(SanbuCraftAI VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(SANBUCRAFT_BUILD_TESTS "Build SanbuCraft AI tests" ON)

add_library(sanbucraft_core
    src/core/Config.cpp
    src/core/Logger.cpp
)

target_include_directories(sanbucraft_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_features(sanbucraft_core PUBLIC cxx_std_17)

if(MSVC)
    target_compile_options(sanbucraft_core PRIVATE /W4 /permissive-)
else()
    target_compile_options(sanbucraft_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(sanbucraft_ai src/main.cpp)
target_link_libraries(sanbucraft_ai PRIVATE sanbucraft_core)

include(CTest)
if(BUILD_TESTING AND SANBUCRAFT_BUILD_TESTS)
    add_executable(config_tests tests/ConfigTests.cpp)
    target_link_libraries(config_tests PRIVATE sanbucraft_core)
    add_test(NAME config_tests COMMAND config_tests)

    add_executable(logger_tests tests/LoggerTests.cpp)
    target_link_libraries(logger_tests PRIVATE sanbucraft_core)
    add_test(NAME logger_tests COMMAND logger_tests)
endif()
