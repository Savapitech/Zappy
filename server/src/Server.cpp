#include <algorithm>
#include <csignal>
#include <exception>
#include <format>

#include <arpa/inet.h>
#include <string>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/wait.h>

#include "Game/Common.hpp"
#include "Game/GameLogic.hpp"
#include "Logger.hpp"
#include "Server.hpp"

Server::Server(uint16_t port, game::GameLogic &game)
    : _socket(port), _game(game) {
  this->_fds.push_back(
      {.fd = this->_socket.getFd(), .events = POLLIN, .revents = 0});
}

void Server::run(game::GameLogic &game) {
  this->_socket.listen();
  signal(SIGCHLD, SIG_IGN);
  signal(SIGPIPE, SIG_IGN);
  int poll_result;

  LOG_INFO(std::format("Zappy Server launched on port [" BLUE "{}" RESET "]",
                       this->_socket.getPort())
               .c_str());

  while (this->_isRunning) {
    if (game.poll()) {
      LOG_INFO("Game over, shutting down server");
      this->stop();
      break;
    }

    for (const auto &client : this->_clients)
      client->tick(game.getFreq());

    poll_result = poll(this->_fds.data(), this->_fds.size(), GAME_TICK_MS);

    if (poll_result < 0) {
      if (errno == EINTR)
        continue;
      throw std::runtime_error("Poll failed");
    }

    if (poll_result == 0)
      continue;

    size_t current_size = this->_fds.size();
    for (size_t i = 1; i < current_size; ++i) {
      if (i >= this->_fds.size())
        break;

      bool shouldDisconnect = false;

      if (this->_fds[i].revents & POLLIN)
        this->handleClientMessage(this->_fds[i].fd);

      if (i >= this->_fds.size())
        break;

      if (this->_fds[i].revents & (POLLHUP | POLLERR | POLLNVAL))
        shouldDisconnect = true;

      auto it = std::find_if(this->_clients.begin(), this->_clients.end(),
                             [&](const std::shared_ptr<Client> &c) {
                               return c->getFd() == this->_fds[i].fd;
                             });
      if (it != this->_clients.end() && !(*it)->isConnected())
        shouldDisconnect = true;

      if (shouldDisconnect) {
        int fd = this->_fds[i].fd;
        this->disconnectClient(fd);
        --i;
        current_size = this->_fds.size();
      }
    }

    if (this->_fds[0].revents & POLLIN)
      this->handleNewConnection();
  }
}

void Server::handleNewConnection() {
  sockaddr_in clientAddr;
  socklen_t clientAddrLen = sizeof(clientAddr);
  int clientFd =
      accept(this->_socket.getFd(), (sockaddr *)&clientAddr, &clientAddrLen);

  if (clientFd < 0) {
    LOG_ERROR("Accept failed");
    return;
  }

  auto newClient = std::make_shared<Client>(clientFd, clientAddr, *this);
  this->_clients.push_back(newClient);
  this->_fds.push_back({.fd = clientFd, .events = POLLIN, .revents = 0});

  LOG_INFO(std::format("New client connected from [{}]",
                       inet_ntoa(clientAddr.sin_addr)));
}

void Server::handleClientMessage(int clientFd) {
  auto it = std::find_if(this->_clients.begin(), this->_clients.end(),
                         [clientFd](const std::shared_ptr<Client> &c) {
                           return c->getFd() == clientFd;
                         });

  if (it != this->_clients.end()) {
    try {
      (*it)->handleMessage();
    } catch (std::exception &e) {
      LOG_WARN(std::format("Client error from {} [{}]",
                           inet_ntoa((*it)->getAddr().sin_addr), e.what()));
    }
  }
}

void Server::disconnectClient(int fd) {
  auto fdIt = std::find_if(this->_fds.begin(), this->_fds.end(),
                           [fd](const pollfd &pfd) { return pfd.fd == fd; });
  if (fdIt != this->_fds.end())
    this->_fds.erase(fdIt);

  auto clientIt = std::find_if(
      this->_clients.begin(), this->_clients.end(),
      [fd](const std::shared_ptr<Client> &c) { return c->getFd() == fd; });

  if (clientIt != this->_clients.end()) {
    auto client = *clientIt;
    auto player = client->getPlayer();

    if (player) {
      for (const auto &team : this->_game.getTeams()) {
        if (team->getName() == player->getTeamname()) {
          team->removeUser(player->getId());
          break;
        }
      }
    }

    LOG_INFO(std::format("Client [{}] disconnected",
                         inet_ntoa(client->getAddr().sin_addr)));
    this->_clients.erase(clientIt);
  }
}

void Server::stop() { this->_isRunning = false; }

void Server::broadcastToGui(const std::string &msg) const {
  for (const auto &client : this->_clients) {
    if (client->getType() == ClientType::GUI)
      client->sendMessage(msg);
  }
}
