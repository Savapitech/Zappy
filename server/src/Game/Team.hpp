#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Player.hpp"

namespace game {
class Team {
private:
  std::string _name;
  int _nbClientMax;
  int _connected;
  std::vector<std::unique_ptr<Player>> _players;

public:
  Team(const std::string &name, int nbClientMax)
      : _name(name), _nbClientMax(nbClientMax), _connected(0) {}

  Team(const Team &) = delete;
  Team &operator=(const Team &) = delete;

  const std::string &getName() const { return _name; }
  int getClientMax() const { return _nbClientMax; }
  int getConnected() const { return _connected; }
  int getAvailable() const { return (_nbClientMax - _connected); }
  const std::vector<std::unique_ptr<Player>> &getPlayers() const {
    return _players;
  }

  void addClientMax() { _nbClientMax++; }
  void addConnected() { _connected++; }
  void removeConnected() { _connected--; }
  void addPlayer(std::unique_ptr<Player> user) {
    _players.push_back(std::move(user));
  }
  void removeUser(Player *user);
};
} // namespace game
