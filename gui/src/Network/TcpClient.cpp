#include "TcpClient.hpp"
#include "../Logger.hpp"

#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <poll.h>

namespace Zappy {

TcpClient::TcpClient() : _socket(-1), _isConnected(false) {
}

TcpClient::~TcpClient() {
    if (_socket != -1) {
        close(_socket);
    }
}

bool TcpClient::connectToServer(const std::string& host, int port) {
    if (_socket != -1) {
        close(_socket);
        _socket = -1;
    }

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
        close(_socket);
        _socket = -1;
        return false;
    }

    if (connect(_socket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        LOG_FATAL("Connection failed");
        close(_socket);
        _socket = -1;
        return false;
    }

    _isConnected = true;
    return true;
}

void TcpClient::sendCommand(const std::string& cmd) {
    if (_isConnected && _socket != -1)
        send(_socket, cmd.c_str(), cmd.size(), 0);
}

std::vector<std::string> TcpClient::fetchLines() {
    std::vector<std::string> lines;

    if (!_isConnected)
        return lines;

    struct pollfd pfd;
    pfd.fd = _socket;
    pfd.events = POLLIN;
    pfd.revents = 0;

    int pollResult = poll(&pfd, 1, 0);

    if (pollResult < 0) {
        _isConnected = false;
        close(_socket);
        _socket = -1;
        LOG_ERROR("Poll error, connection lost.");
        return lines;
    }

    if (pollResult > 0 && (pfd.revents & POLLIN)) {
        char tempBuffer[4096];
        ssize_t bytesRead = recv(_socket, tempBuffer, sizeof(tempBuffer) - 1, 0);

        if (bytesRead > 0) {
            tempBuffer[bytesRead] = '\0';
            _buffer += tempBuffer;

            size_t pos;
            while ((pos = _buffer.find('\n')) != std::string::npos) {
                lines.push_back(_buffer.substr(0, pos));
                _buffer.erase(0, pos + 1);
            }
        } 
        else if (bytesRead == 0) {
            _isConnected = false;
            close(_socket);
            _socket = -1;
            LOG_WARN("Server closed the connection.");
        }
    }

    return lines;
}

} // namespace Zappy