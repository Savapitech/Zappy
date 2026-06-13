#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>

#include "../GameState.hpp"
#include "NetworkEvent.hpp"
#include "INetworkClient.hpp"

namespace Zappy {

class NetworkManager {
public:
    NetworkManager(INetworkClient& client);
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
    INetworkClient& _netClient;

    std::unordered_map<std::string,  std::function<void(const std::vector<std::string>&)>> _commandHandlers;
    void initCommandHandlers();

    void processLine(const std::string& line);
    std::vector<std::string> splitString(const std::string& str, char delimiter);
    int parseId(const std::string& idStr);

    void handleWelcome();
    void handleMsz(const std::vector<std::string>& args);
    void handleBct(const std::vector<std::string>& args);
    void handleTna(const std::vector<std::string>& args);
    void handlePnw(const std::vector<std::string>& args);
    void handlePpo(const std::vector<std::string>& args);
    void handlePlv(const std::vector<std::string>& args);
    void handlePin(const std::vector<std::string>& args);
    void handlePdi(const std::vector<std::string>& args);
    void handleSgt(const std::vector<std::string>& args);
    void handlePbc(const std::vector<std::string>& args);
    void handlePic(const std::vector<std::string>& args);
    void handleSeg(const std::vector<std::string>& args);
    void handleEnw(const std::vector<std::string>& args);
    void handlePgt(const std::vector<std::string>& args);
    void handlePdr(const std::vector<std::string>& args);
    void handleEbo(const std::vector<std::string>& args);
    void handleEdi(const std::vector<std::string>& args);
    void handlePex(const std::vector<std::string>& args);
    void handlePie(const std::vector<std::string>& args);
    void handlePfk(const std::vector<std::string>& args);
    void handleSst(const std::vector<std::string>& args);
    void handleSmg(const std::vector<std::string>& args);
    void handleSuc(const std::vector<std::string>& args);
    void handleSbp(const std::vector<std::string>& args);
};

} // namespace Zappy