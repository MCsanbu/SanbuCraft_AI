#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace sanbucraft::minecraft {
struct ItemStack { std::string id; int count = 0; int slot = -1; };
struct Player { std::string uuid; std::string name; std::string dimension = "minecraft:overworld"; double x = 0; double y = 0; double z = 0; double health = 20; int foodLevel = 20; int xpLevel = 0; float xpProgress = 0; int selectedSlot = 0; std::vector<ItemStack> inventory, armor, enderChest; };
struct Container { std::string id; std::string dimension; int x = 0, y = 0, z = 0; std::string type; std::vector<ItemStack> items; };
struct WorldInfo { std::string path, name, versionName; std::int64_t seed = 0, gameTime = 0; int dataVersion = 0; std::map<std::string, std::string> gameRules; };
}
