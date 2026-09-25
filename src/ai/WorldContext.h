#pragma once

#include "analysis/WorldAnalyzer.h"

#include <string>
#include <vector>

namespace sanbucraft::ai {

struct WorldContext {
    minecraft::WorldInfo world;
    std::vector<minecraft::Player> players;
    std::vector<minecraft::Container> containers;
    analysis::WorldAnalysis analysis;

    std::string toPrompt() const;
};

WorldContext buildWorldContext(const minecraft::WorldInfo& world,
                               const std::vector<minecraft::Player>& players,
                               const std::vector<minecraft::Container>& containers);

}  // namespace sanbucraft::ai
