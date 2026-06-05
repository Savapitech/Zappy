#pragma once

#include <string>
#include <vector>
#include <memory>
#include "../GameState.hpp"
#include "NetworkEvent.hpp"

namespace Zappy {

class NetworkManager {
public:
    NetworkManager();
    ~NetworkManager();

    bool connectToServer(const std::string& host, int port);
    void update();
    void sendCommand(const std::string& cmd);

    const GameState& getGameState() const { return _gameState; }
    std::vector<NetworkEvent> consumeEvents();

private:
    int _socket;
    bool _isConnected;
    std::string _buffer;

    GameState _gameState;
    std::vector<NetworkEvent> _eventQueue;

    void processLine(const std::string& line);
    std::vector<std::string> splitString(const std::string& str, char delimiter);

    void handleWelcome();
    void handleMsz(const std::vector<std::string>& args);
    void handlePnw(const std::vector<std::string>& args);
};

} // namespace Zappy