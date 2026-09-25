#include "agent/ActionExecutor.h"

#include <iostream>

namespace {
class FakeConnection final : public sanbucraft::network::MinecraftConnection {
public:
    bool connect(std::string&) override { connected = true; return true; }
    void disconnect() override { connected = false; }
    bool isConnected() const override { return connected; }
    bool sendCommand(const std::string& command, std::string& response, std::string&) override { response = "ran: " + command; return true; }
    bool connected = false;
};
}

int main() {
    FakeConnection connection;
    sanbucraft::agent::ActionExecutor executor(connection);
    const auto unauthenticated = executor.prepare("Locate village", "locate structure minecraft:village");
    if (executor.confirmAndExecute(unauthenticated.id).success) return 1;
    std::string error;
    connection.connect(error);
    const auto allowed = executor.prepare("Locate village", "locate structure minecraft:village");
    if (!executor.confirmAndExecute(allowed.id).success) return 1;
    const auto denied = executor.prepare("Dangerous", "fill ~ ~ ~ ~ ~ ~ air");
    if (executor.confirmAndExecute(denied.id).success) return 1;
    return 0;
}
