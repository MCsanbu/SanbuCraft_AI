#pragma once

#include "network/MinecraftConnection.h"

#include <chrono>
#include <map>
#include <string>

namespace sanbucraft::agent {

struct ActionPlan {
    std::string id;
    std::string description;
    std::string command;
    std::chrono::system_clock::time_point expiresAt;
};

struct ActionResult {
    bool success = false;
    std::string message;
};

class ActionExecutor {
public:
    explicit ActionExecutor(network::MinecraftConnection& connection);

    ActionPlan prepare(const std::string& description, const std::string& command);
    ActionResult confirmAndExecute(const std::string& planId);
    void cancel(const std::string& planId);

private:
    static bool isSafeCommand(const std::string& command);
    static std::string createId();

    network::MinecraftConnection& connection_;
    std::map<std::string, ActionPlan> pendingPlans_;
};

}  // namespace sanbucraft::agent
