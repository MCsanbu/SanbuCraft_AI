#include "ai/Agent.h"
#include "ai/MockAIProvider.h"

#include <iostream>

int main() {
    sanbucraft::minecraft::WorldInfo world;
    world.name = "Test";
    sanbucraft::minecraft::Player player;
    player.inventory.push_back({"minecraft:diamond", 12, 0});
    const auto context = sanbucraft::ai::buildWorldContext(world, {player}, {});
    sanbucraft::ai::MockAIProvider provider;
    sanbucraft::ai::Agent agent(provider);
    if (agent.ask(context, "How many diamonds?").answer.find("12") == std::string::npos) return 1;
    if (!agent.ask(context, "帮我整理仓库").requiresConfirmation) return 1;
    return 0;
}
