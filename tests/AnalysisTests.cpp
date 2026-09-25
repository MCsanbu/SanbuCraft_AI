#include "analysis/WorldAnalyzer.h"
#include <iostream>
int main() { sanbucraft::minecraft::Player player; player.inventory.push_back({"minecraft:diamond", 7, 0}); player.inventory.push_back({"minecraft:bread", 3, 1}); auto result = sanbucraft::analysis::WorldAnalyzer::analyze({player}, {}); if (result.items["minecraft:diamond"] != 7 || result.food != 3) { std::cerr << "Analysis mismatch\n"; return 1; } return 0; }
