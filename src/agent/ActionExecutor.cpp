#include "agent/ActionExecutor.h"

#include <atomic>
#include <chrono>

namespace sanbucraft::agent {

ActionExecutor::ActionExecutor(network::MinecraftConnection& connection) : connection_(connection) {}

ActionPlan ActionExecutor::prepare(const std::string& description, const std::string& command) {
    ActionPlan plan{createId(), description, command, std::chrono::system_clock::now() + std::chrono::minutes(5)};
    pendingPlans_[plan.id] = plan;
    return plan;
}

ActionResult ActionExecutor::confirmAndExecute(const std::string& planId) {
    const auto iterator = pendingPlans_.find(planId);
    if (iterator == pendingPlans_.end()) return {false, "Action plan was not found or was already consumed."};
    const ActionPlan plan = iterator->second;
    pendingPlans_.erase(iterator);
    if (std::chrono::system_clock::now() > plan.expiresAt) return {false, "Action plan expired; create and review a new plan."};
    if (!isSafeCommand(plan.command)) return {false, "Command rejected by the safety allow-list."};
    if (!connection_.isConnected()) return {false, "Minecraft connection is not authenticated; no command was sent."};
    std::string response;
    std::string error;
    if (!connection_.sendCommand(plan.command, response, error)) return {false, "Command failed: " + error};
    return {true, response.empty() ? "Command completed." : response};
}

void ActionExecutor::cancel(const std::string& planId) { pendingPlans_.erase(planId); }

bool ActionExecutor::isSafeCommand(const std::string& command) {
    // Only non-destructive informational commands are enabled by default.
    return command.rfind("list", 0) == 0 || command.rfind("data get", 0) == 0 || command.rfind("locate", 0) == 0;
}

std::string ActionExecutor::createId() {
    static std::atomic<unsigned long long> sequence{0};
    return "action-" + std::to_string(++sequence);
}

}  // namespace sanbucraft::agent
