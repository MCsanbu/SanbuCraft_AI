#pragma once
#include "minecraft/Models.h"
#include <string>
#include <vector>
namespace sanbucraft::minecraft { class ContainerScanner { public: static bool scan(const std::string& worldPath, std::vector<Container>& containers, std::string& error); }; }
