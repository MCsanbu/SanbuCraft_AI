#pragma once
#include "minecraft/Models.h"
#include <string>
namespace sanbucraft::minecraft { class WorldLoader { public: static bool loadWorld(const std::string& path, WorldInfo& world, std::string& error); }; }
