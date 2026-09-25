#pragma once
#include "analysis/WorldAnalyzer.h"
#include "minecraft/Models.h"
#include <string>
#include <vector>
struct sqlite3;
namespace sanbucraft::database { class Database { public: Database() = default; ~Database(); Database(const Database&) = delete; bool open(const std::string& path, std::string& error); bool migrate(std::string& error); bool replaceWorldData(const sanbucraft::minecraft::WorldInfo& world, const std::vector<sanbucraft::minecraft::Player>& players, const std::vector<sanbucraft::minecraft::Container>& containers, std::string& error); private: sqlite3* connection_ = nullptr; bool execute(const char* sql, std::string& error); }; }
