#pragma once

#include "ai/AIProvider.h"
#include "ai/WorldContext.h"

#include <string>

namespace sanbucraft::ai {

struct AgentReply {
    std::string answer;
    bool requiresConfirmation = false;
    std::string proposedAction;
};

class Agent {
public:
    explicit Agent(AIProvider& provider);
    AgentReply ask(const WorldContext& context, const std::string& question);

private:
    static std::string normalizeItemId(const std::string& question);
    static bool requestsWorldModification(const std::string& question);

    AIProvider& provider_;
};

}  // namespace sanbucraft::ai
