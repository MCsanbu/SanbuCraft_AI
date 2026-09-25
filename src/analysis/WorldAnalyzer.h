#pragma once
#include "minecraft/Models.h"
#include <map>
#include <string>
#include <vector>
namespace sanbucraft::analysis { struct WorldAnalysis { std::map<std::string, int> items; int food = 0; int containers = 0; }; class WorldAnalyzer { public: static WorldAnalysis analyze(const std::vector<sanbucraft::minecraft::Player>& players, const std::vector<sanbucraft::minecraft::Container>& containers); static std::vector<sanbucraft::minecraft::Container> findContainers(const std::vector<sanbucraft::minecraft::Container>& containers, const std::string& itemId); }; }
