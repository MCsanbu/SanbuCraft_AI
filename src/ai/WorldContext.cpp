#include "ai/WorldContext.h"

#include <sstream>

namespace sanbucraft::ai {

WorldContext buildWorldContext(const minecraft::WorldInfo& world,
                               const std::vector<minecraft::Player>& players,
                               const std::vector<minecraft::Container>& containers) {
    return {world, players, containers, analysis::WorldAnalyzer::analyze(players, containers)};
}

std::string WorldContext::toPrompt() const {
    std::ostringstream prompt;
    prompt << "World: " << world.name << "\nSeed: " << world.seed << "\nGame time: " << world.gameTime << "\n";
    prompt << "Players: " << players.size() << "\nContainers: " << containers.size() << "\nFood items: " << analysis.food << "\nResources:\n";
    for (const auto& [item, count] : analysis.items) prompt << "- " << item << ": " << count << "\n";
    for (const auto& player : players) {
        prompt << "Player " << player.uuid << ": level " << player.xpLevel << ", health " << player.health
               << ", food " << player.foodLevel << ", position " << player.x << ", " << player.y << ", " << player.z
               << ", dimension " << player.dimension << "\n";
    }
    return prompt.str();
}

}  // namespace sanbucraft::ai
