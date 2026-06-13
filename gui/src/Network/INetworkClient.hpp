#pragma once

#include <string>
#include <vector>

class INetworkClient {
public:
    virtual ~INetworkClient() = default;
    virtual bool connectToServer(const std::string& host, int port) = 0;
    virtual void sendCommand(const std::string& cmd) = 0;
    virtual std::vector<std::string> fetchLines() = 0;
};