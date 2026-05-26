#pragma once

#include <string>
#include <vector>
#include <memory>

#include "Player.hpp"


namespace game {
  class Team {
    private: 
      std::string _name;
      int _nbClientMax;
      int _connected;
      std::vector<std::unique_ptr<User>> _users;

    public:
      Team(const std::string &name, int nbClientMax) : _name(name), _nbClientMax(nbClientMax), _connected(0) {}

      Team(const Team &) = delete;
      Team &operator=(const Team &) = delete;

      const std::string &getName() const { return _name; }
      int getClientMax() const { return _nbClientMax; }
      int getConnected() const { return _connected; }
      int getAvailable() const { return (_nbClientMax - _connected); }
      const std::vector<std::unique_ptr<User>> &getUsers() const { return _users; }

      void addClientMax() { _nbClientMax++; }
      void addConnected() { _connected++; }
      void removeConnected() { _connected--; }
      void addUser(std::unique_ptr<User> user) { _users.push_back(std::move(user)); }
      void removeUser(User *user);
  };
} //namespace game
