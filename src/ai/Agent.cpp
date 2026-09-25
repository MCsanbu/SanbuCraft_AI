#include "ai/Agent.h"

#include <algorithm>
#include <cctype>
#include <sstream>

namespace sanbucraft::ai {
namespace {
std::string lower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char character) { return static_cast<char>(std::tolower(character)); });
    return value;
}
}

Agent::Agent(AIProvider& provider) : provider_(provider) {}

AgentReply Agent::ask(const WorldContext& context, const std::string& question) {
    if (requestsWorldModification(question)) {
        return {"This request could modify the Minecraft world. I have not sent any command.", true,
                "Review and explicitly confirm a future action plan for: " + question};
    }

    const std::string item = normalizeItemId(question);
    if (!item.empty()) {
        const auto count = context.analysis.items.find(item);
        std::ostringstream answer;
        answer << item << ": " << (count == context.analysis.items.end() ? 0 : count->second);
        const auto locations = analysis::WorldAnalyzer::findContainers(context.containers, item);
        if (!locations.empty()) {
            answer << "\nFound in containers:";
            for (const auto& container : locations) answer << "\n- " << container.type << " at " << container.x << ", " << container.y << ", " << container.z;
        }
        return {answer.str(), false, ""};
    }

    const ChatResponse response = provider_.chat({context.toPrompt(), question});
    return {response.success ? response.content : "AI provider error: " + response.error, false, ""};
}

std::string Agent::normalizeItemId(const std::string& question) {
    const std::string text = lower(question);
    const std::pair<const char*, const char*> knownItems[] = {
        {"diamond", "minecraft:diamond"}, {"钻石", "minecraft:diamond"},
        {"iron", "minecraft:iron_ingot"}, {"铁", "minecraft:iron_ingot"},
        {"gold", "minecraft:gold_ingot"}, {"金", "minecraft:gold_ingot"},
        {"coal", "minecraft:coal"}, {"煤", "minecraft:coal"},
        {"emerald", "minecraft:emerald"}, {"绿宝石", "minecraft:emerald"},
        {"bread", "minecraft:bread"}, {"面包", "minecraft:bread"}};
    for (const auto& [term, item] : knownItems) if (text.find(term) != std::string::npos) return item;
    return "";
}

bool Agent::requestsWorldModification(const std::string& question) {
    const std::string text = lower(question);
    const char* dangerousTerms[] = {"整理", "放置", "建造", "删除", "执行", "organize", "place", "build", "delete", "execute"};
    for (const char* term : dangerousTerms) if (text.find(term) != std::string::npos) return true;
    return false;
}

}  // namespace sanbucraft::ai
