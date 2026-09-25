#include "network/RconConnection.h"

namespace sanbucraft::network {

RconConnection::RconConnection(std::string host, const unsigned short port) : host_(std::move(host)), port_(port) {}

bool RconConnection::connect(std::string& error) {
    error = "RCON transport is intentionally disabled until authenticated socket support and credential storage are configured.";
    connected_ = false;
    return false;
}

void RconConnection::disconnect() { connected_ = false; }
bool RconConnection::isConnected() const { return connected_; }

bool RconConnection::sendCommand(const std::string&, std::string&, std::string& error) {
    error = "RCON command execution is unavailable without an authenticated connection.";
    return false;
}

}  // namespace sanbucraft::network
