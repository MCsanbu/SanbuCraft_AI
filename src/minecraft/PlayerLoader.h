#pragma once
#include "minecraft/Models.h"
#include <string>
#include <vector>
namespace sanbucraft::minecraft { class PlayerLoader { public: static bool loadPlayers(const std::string& worldPath, std::vector<Player>& players, std::string& error); }; }
