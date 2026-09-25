#pragma once

#include <string>

namespace sanbucraft::network {

class MinecraftConnection {
public:
    virtual ~MinecraftConnection() = default;
    virtual bool connect(std::string& error) = 0;
    virtual void disconnect() = 0;
    virtual bool isConnected() const = 0;
    virtual bool sendCommand(const std::string& command, std::string& response, std::string& error) = 0;
};

}  // namespace sanbucraft::network
