#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>

#define close closesocket
#define poll WSAPoll
typedef int socklen_t;

#pragma comment(lib, "ws2_32.lib")

#else
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#endif

#include "Logger.hpp"
#include "TcpClient.hpp"
#include <iostream>

namespace Zappy {

TcpClient::TcpClient() : _socket(INVALID_SOCKET), _isConnected(false) {
#ifdef _WIN32

  WSADATA wsaData;
  if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
    LOG_ERROR("WSAStartup failed on Windows");
  }
#endif
}

TcpClient::~TcpClient() {
  if (_socket != INVALID_SOCKET) {
    close(_socket);
  }
#ifdef _WIN32
  WSACleanup();
#endif
}

bool TcpClient::connectToServer(const std::string &host, int port) {
  if (_socket != INVALID_SOCKET) {
    close(_socket);
    _socket = INVALID_SOCKET;
  }

  _socket = socket(AF_INET, SOCK_STREAM, 0);
  if (_socket == INVALID_SOCKET) {
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

  if (connect(_socket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) <
      0) {
    LOG_FATAL("Connection failed");
    close(_socket);
    _socket = INVALID_SOCKET;
    return false;
  }

  _isConnected = true;
  return true;
}

void TcpClient::sendCommand(const std::string &cmd) {
  if (_isConnected && _socket != INVALID_SOCKET)
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
    int bytesRead = recv(_socket, tempBuffer, sizeof(tempBuffer) - 1, 0);

    if (bytesRead > 0) {
      tempBuffer[bytesRead] = '\0';
      _buffer += tempBuffer;

      size_t pos;
      while ((pos = _buffer.find('\n')) != std::string::npos) {
        lines.push_back(_buffer.substr(0, pos));
        _buffer.erase(0, pos + 1);
      }
    } else if (bytesRead == 0) {
      _isConnected = false;
      close(_socket);
      _socket = INVALID_SOCKET;
      LOG_WARN("Server closed the connection.");
    }
  }

  return lines;
}

} // namespace Zappy