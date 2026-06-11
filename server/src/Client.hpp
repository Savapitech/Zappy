#pragma once

#include <functional>
#include <map>
#include <memory>
#include <netinet/in.h>
#include <string>

#include "Commands/ICommand.hpp"

namespace game {
class Player;
}

class Server;

class Client : public std::enable_shared_from_this<Client> {
private:
  int _fd;
  sockaddr_in _addr;
  bool _isConnected = true;
  std::string _buffer;
  std::reference_wrapper<Server> _server;
  std::map<std::string, std::shared_ptr<commands::ICommand>> _commands;
  std::shared_ptr<game::Player> _player;

private:
  void registerCommands();

public:
  Client(int fd, sockaddr_in addr, std::reference_wrapper<Server> server);
  ~Client();

  void close();
  int getFd() const;
  sockaddr_in getAddr() const;
  void setPlayer(std::shared_ptr<game::Player> player) { _player = player; }
  std::shared_ptr<game::Player> getPlayer() { return _player; }

  void handleMessage();
  void sendMessage(const std::string &msg);
  bool isConnected() const;
  void disconnect();
  std::reference_wrapper<Server> getServer();

private:
  void processCommand(const std::string &commandLine);
};
