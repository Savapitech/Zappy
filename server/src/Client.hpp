#pragma once

#include <chrono>
#include <deque>
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

enum class ClientType { AI, GUI };

class Client : public std::enable_shared_from_this<Client> {
private:
  struct PendingCommand {
    std::shared_ptr<commands::ICommand> command;
    std::vector<std::string> args;
  };

  int _fd;
  sockaddr_in _addr;
  bool _isConnected = true;
  bool _handshakeDone = false;
  ClientType _type = ClientType::AI;
  std::string _buffer;
  std::reference_wrapper<Server> _server;
  std::map<std::string, std::shared_ptr<commands::ICommand>> _aiCommands;
  std::map<std::string, std::shared_ptr<commands::ICommand>> _guiCommands;
  std::shared_ptr<game::Player> _player;
  std::deque<PendingCommand> _pendingCommands;
  bool _executingCommand = false;
  std::chrono::steady_clock::time_point _commandReadyAt;

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
  void setType(ClientType type) { _type = type; }
  ClientType getType() const { return _type; }

  void handleMessage();
  void sendMessage(const std::string &msg);
  bool isConnected() const;
  void disconnect();
  std::reference_wrapper<Server> getServer();
  void tick(int freq);

private:
  void processCommand(const std::string &commandLine);
};
