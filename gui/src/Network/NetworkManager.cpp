#include "NetworkManager.hpp"
#include <iostream>
#include <sstream>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <poll.h>

#include "Logger.hpp"

namespace Zappy {

NetworkManager::NetworkManager() : _socket(-1), _isConnected(false) 
    {
        initCommandHandlers();
    }

NetworkManager::~NetworkManager() {
    if (_socket != -1) {
        close(_socket);
    }
}

void NetworkManager::initCommandHandlers() {
    _commandHandlers["msz"] = [this](const auto& args) { handleMsz(args); };
    _commandHandlers["bct"] = [this](const auto& args) { handleBct(args); };
    _commandHandlers["tna"] = [this](const auto& args) { handleTna(args); };
    _commandHandlers["sgt"] = [this](const auto& args) { handleSgt(args); };
    _commandHandlers["pnw"] = [this](const auto& args) { handlePnw(args); };
    _commandHandlers["ppo"] = [this](const auto& args) { handlePpo(args); };
    _commandHandlers["plv"] = [this](const auto& args) { handlePlv(args); };
    _commandHandlers["pin"] = [this](const auto& args) { handlePin(args); };
    _commandHandlers["pdi"] = [this](const auto& args) { handlePdi(args); };
    _commandHandlers["pbc"] = [this](const auto& args) { handlePbc(args); };
    _commandHandlers["pic"] = [this](const auto& args) { handlePic(args); };
    _commandHandlers["seg"] = [this](const auto& args) { handleSeg(args); };
}

bool NetworkManager::connectToServer(const std::string& host, int port) {
    _socket = socket(AF_INET, SOCK_STREAM, 0);
    if (_socket == -1) {
        LOG_FATAL("Can't open a socket");
        return false;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);

    if (inet_pton(AF_INET, host.c_str(), &serverAddr.sin_addr) <= 0) {
        LOG_FATAL("Invalid IP");
        return false;
    }

    if (connect(_socket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        LOG_FATAL("Connection failed");
        return false;
    }

    int flags = fcntl(_socket, F_GETFL, 0);
    fcntl(_socket, F_SETFL, flags | O_NONBLOCK);

    _isConnected = true;
    return true;
}

void NetworkManager::update() {
    if (!_isConnected) return;

    struct pollfd pfd;
    pfd.fd = _socket;
    pfd.events = POLLIN;
    pfd.revents = 0;

    int pollResult = poll(&pfd, 1, 0);

    if (pollResult < 0) {
        _isConnected = false;
        close(_socket);
        _socket = -1;
        return;
    }

    if (pollResult == 0) return; 

    if (pfd.revents & POLLIN) {
        char tempBuffer[4096];
        ssize_t bytesRead = recv(_socket, tempBuffer, sizeof(tempBuffer) - 1, 0);

        if (bytesRead > 0) {
            tempBuffer[bytesRead] = '\0';
            _buffer += tempBuffer;

            size_t pos;
            while ((pos = _buffer.find('\n')) != std::string::npos) {
                std::string line = _buffer.substr(0, pos);
                _buffer.erase(0, pos + 1);
                processLine(line);
            }
        } 
        else if (bytesRead == 0) {
            _isConnected = false;
            close(_socket);
            _socket = -1;
        }
    }
}

void NetworkManager::sendCommand(const std::string& cmd) {
    if (_isConnected) {
        send(_socket, cmd.c_str(), cmd.size(), 0);
    }
}

std::vector<NetworkEvent> NetworkManager::consumeEvents() {
    auto events = std::move(_eventQueue);
    _eventQueue.clear();
    return events;
}

std::vector<std::string> NetworkManager::splitString(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);
    while (std::getline(tokenStream, token, delimiter)) {
        if (!token.empty()) tokens.push_back(token);
    }
    return tokens;
}

int NetworkManager::parseId(const std::string& idStr) {
    if (idStr.empty()) return 0;
    return (idStr[0] == '#') ? std::stoi(idStr.substr(1)) : std::stoi(idStr);
}

void NetworkManager::processLine(const std::string& line) {
    if (line == "WELCOME") 
    {
        handleWelcome();
        return; 
    }

    auto args = splitString(line, ' ');
    if (args.empty()) return;

    const std::string &cmd = args[0];

    auto it = _commandHandlers.find(cmd);
    if (it != _commandHandlers.end()) {
        it->second(args);
    } else {
        LOG_WARN("UNKNOWN_COMMAND");
    }
}

void NetworkManager::handleWelcome() {
    sendCommand("GRAPHIC\n");
}

void NetworkManager::handleMsz(const std::vector<std::string>& args) {
    if (args.size() >= 3) {
        _gameState.map.width = std::stoi(args[1]);
        _gameState.map.height = std::stoi(args[2]);
        _gameState.grid.resize(_gameState.map.width * _gameState.map.height);
        _gameState.map.isInitialized = true;

        sendCommand("mct\n"); 
    }
}

void NetworkManager::handleBct(const std::vector<std::string>& args) {
    if (args.size() >= 10 && _gameState.map.isInitialized) {
        int x = std::stoi(args[1]);
        int y = std::stoi(args[2]);
        int index = y * _gameState.map.width + x;
        if (index >= 0 && static_cast<size_t>(index) < _gameState.grid.size()) {
            for (int i = 0; i < 7; ++i) {
                _gameState.grid[index].resources[i] = std::stoi(args[3 + i]);
            }
            _eventQueue.push_back({NetworkEventType::TILE_UPDATED, args});
        }
    }
}

void NetworkManager::handleTna(const std::vector<std::string>& args) {
    if (args.size() >= 2) {
        _gameState.map.teamNames.push_back(args[1]);
    }
}

void NetworkManager::handleSgt(const std::vector<std::string>& args) {
    if (args.size() >= 2) {
        _gameState.map.timeUnit = std::stoi(args[1]);
    }
}

void NetworkManager::handlePnw(const std::vector<std::string>& args) {
    if (args.size() >= 7) {
        int id = parseId(args[1]);
        Player p;
        p.id = id;
        p.x = std::stoi(args[2]);
        p.y = std::stoi(args[3]);
        p.orientation = std::stoi(args[4]);
        p.level = std::stoi(args[5]);
        p.team = args[6];
        
        _gameState.players[id] = p;
        _eventQueue.push_back({NetworkEventType::PLAYER_CONNECTED, args});
    }
}

void NetworkManager::handlePpo(const std::vector<std::string>& args) {
    if (args.size() >= 5) {
        int id = parseId(args[1]);
        if (_gameState.players.contains(id)) {
            _gameState.players[id].x = std::stoi(args[2]);
            _gameState.players[id].y = std::stoi(args[3]);
            _gameState.players[id].orientation = std::stoi(args[4]);
        }
    }
}

void NetworkManager::handlePlv(const std::vector<std::string>& args) {
    if (args.size() >= 3) {
        int id = parseId(args[1]);
        if (_gameState.players.contains(id)) {
            _gameState.players[id].level = std::stoi(args[2]);
        }
    }
}

void NetworkManager::handlePin(const std::vector<std::string>& args) {
    if (args.size() >= 11) {
        int id = parseId(args[1]);
        if (_gameState.players.contains(id)) {
            for (int i = 0; i < 7; ++i) {
                _gameState.players[id].inventory[i] = std::stoi(args[4 + i]);
            }
        }
    }
}

void NetworkManager::handlePdi(const std::vector<std::string>& args) {
    if (args.size() >= 2) {
        int id = parseId(args[1]);
        _gameState.players.erase(id);
        _eventQueue.push_back({NetworkEventType::PLAYER_DISCONNECTED, args});
    }
}

void NetworkManager::handlePbc(const std::vector<std::string>& args) {
    _eventQueue.push_back({NetworkEventType::BROADCAST, args});
}

void NetworkManager::handlePic(const std::vector<std::string>& args) {
    _eventQueue.push_back({NetworkEventType::INCANTATION_START, args});
}

void NetworkManager::handleSeg(const std::vector<std::string>& args) {
    _eventQueue.push_back({NetworkEventType::GAME_OVER, args});
}

} // namespace Zappy