#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include <sys/poll.h>

#include "Client.hpp"
#include "Game/GameLogic.hpp"
#include "Socket.hpp"

class Server : public std::enable_shared_from_this<Server> {
private:
  Socket _socket;
  std::vector<pollfd> _fds;
  std::vector<std::shared_ptr<Client>> _clients;
  bool _isRunning = true;
  game::GameLogic &_game;

public:
  Server(uint16_t port, game::GameLogic &game);
  void run(game::GameLogic &game);
  void handleNewConnection();
  void handleClientMessage(int clientFd);
  void disconnectClient(int fd);
  void stop();
  void broadcastToGui(const std::string &msg) const;
  const std::vector<std::shared_ptr<Client>> &getClients() const {
    return _clients;
  }
  game::GameLogic &getGame() const { return _game; }
};
