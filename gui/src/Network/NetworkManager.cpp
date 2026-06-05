#include "NetworkManager.hpp"
#include <iostream>
#include <sstream>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>

#include "Logger.hpp"

namespace Zappy {

NetworkManager::NetworkManager() : _socket(-1), _isConnected(false) {}

NetworkManager::~NetworkManager() {
    if (_socket != -1) {
        close(_socket);
    }
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
    } else if (bytesRead == 0) {
        LOG_FATAL("Connection failed");
        _isConnected = false;
        close(_socket);
        _socket = -1;
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

void NetworkManager::processLine(const std::string& line) {
    if (line == "WELCOME") {
        handleWelcome();
        return;
    }

    auto args = splitString(line, ' ');
    if (args.empty()) return;

    const std::string &cmd = args[0];

    if (cmd == "msz") {
        handleMsz(args);
    } else if (cmd == "pnw") {
        handlePnw(args);
    } 


    else if (cmd == "pbc") {
        _eventQueue.push_back({NetworkEventType::BROADCAST, args});
    } else if (cmd == "pic") {
        _eventQueue.push_back({NetworkEventType::INCANTATION_START, args});
    } else if (cmd == "pdi") {
        _eventQueue.push_back({NetworkEventType::PLAYER_DEATH, args});
    }
}

void NetworkManager::handleWelcome() {
    LOG_INFO("Connection success");
    sendCommand("GRAPHIC\n");
}

void NetworkManager::handleMsz(const std::vector<std::string>& args) {
    if (args.size() >= 3) {
        _gameState.map.width = std::stoi(args[1]);
        _gameState.map.height = std::stoi(args[2]);
        _gameState.grid.resize(_gameState.map.width * _gameState.map.height);
        _gameState.map.isInitialized = true;
    }
}

void NetworkManager::handlePnw(const std::vector<std::string>& args) {
    if (args.size() >= 7) {
        std::string idStr = args[1];
        if (idStr[0] == '#')
            idStr = idStr.substr(1);
        
        int id = std::stoi(idStr);
        Player p;
        p.id = id;
        p.x = std::stoi(args[2]);
        p.y = std::stoi(args[3]);
        p.orientation = std::stoi(args[4]);
        p.level = std::stoi(args[5]);
        p.team = args[6];
        
        _gameState.players[id] = p;
    }
}

} // namespace Zappy