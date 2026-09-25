#pragma once

#include "network/MinecraftConnection.h"

namespace sanbucraft::network {

class RconConnection final : public MinecraftConnection {
public:
    RconConnection(std::string host, unsigned short port);
    bool connect(std::string& error) override;
    void disconnect() override;
    bool isConnected() const override;
    bool sendCommand(const std::string& command, std::string& response, std::string& error) override;

private:
    std::string host_;
    unsigned short port_;
    bool connected_ = false;
};

}  // namespace sanbucraft::network
