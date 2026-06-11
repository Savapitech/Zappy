#include <arpa/inet.h>
#include <stdexcept>
#include <unistd.h>

#include "Client.hpp"
#include "Commands/Broadcast.hpp"
#include "Commands/ConnectNbr.hpp"
#include "Commands/Eject.hpp"
#include "Commands/Forward.hpp"
#include "Commands/Inventory.hpp"
#include "Commands/Left.hpp"
#include "Commands/Right.hpp"
#include "Commands/Set.hpp"
#include "Commands/Take.hpp"
#include "Logger.hpp"
#include "Parser.hpp"

void Client::registerCommands() {
  this->_commands["Forward"] = std::make_shared<commands::Forward>();
  this->_commands["Right"] = std::make_shared<commands::Right>();
  this->_commands["Left"] = std::make_shared<commands::Left>();
  this->_commands["Inventory"] = std::make_shared<commands::Inventory>();
  this->_commands["Broadcast"] = std::make_shared<commands::Broadcast>();
  this->_commands["Connect_nbr"] = std::make_shared<commands::ConnectNbr>();
  this->_commands["Eject"] = std::make_shared<commands::Eject>();
  this->_commands["Take"] = std::make_shared<commands::Take>();
  this->_commands["Set"] = std::make_shared<commands::Set>();
}

Client::Client(int fd, sockaddr_in addr, std::reference_wrapper<Server> server)
    : _fd(fd), _addr(addr), _server(server) {
  registerCommands();
}

Client::~Client() {
  try {
    this->close();
  } catch (const std::exception &e) {
    LOG_ERROR(e.what());
  }
}

void Client::close() {
  if (this->_fd < 0)
    return;

  if (::close(this->_fd) < 0)
    LOG_ERROR("Socket close failed");
  this->_fd = -1;
  this->_isConnected = false;
}

void Client::disconnect() { this->_isConnected = false; }

int Client::getFd() const { return this->_fd; }

sockaddr_in Client::getAddr() const { return this->_addr; }

bool Client::isConnected() const { return this->_isConnected; }

std::reference_wrapper<Server> Client::getServer() { return this->_server; }

void Client::sendMessage(const std::string &msg) {
  if (this->_fd < 0)
    return;
  write(this->_fd, msg.c_str(), msg.length());
  LOG_DEBUG("Message sent to client [" + msg + "]");
}

void Client::handleMessage() {
  char buffer[4096];
  ssize_t bytesRead = ::read(this->_fd, buffer, sizeof(buffer) - 1);

  if (bytesRead <= 0) {
    this->_isConnected = false;
    return;
  }

  buffer[bytesRead] = '\0';
  this->_buffer += buffer;

  size_t pos;
  while ((pos = this->_buffer.find("\n")) != std::string::npos) {
    std::string commandLine = this->_buffer.substr(0, pos);
    this->_buffer.erase(0, pos + 1);

    if (!commandLine.empty() && commandLine.back() == '\r')
      commandLine.pop_back();

    if (!commandLine.empty())
      this->processCommand(commandLine);
  }
}

void Client::processCommand(const std::string &commandLine) {
  LOG_DEBUG(std::format("Received from {} [{}]",
                        inet_ntoa(this->_addr.sin_addr), commandLine));

  std::string cmd = commandLine.substr(0, commandLine.find_first_of(" \t"));
  std::vector<std::string> args = ParseArgs(commandLine.substr(cmd.size()));

  LOG_DEBUG(
      std::format("Parsed command name [{}] args size [{}]", cmd, args.size()));

  auto it = this->_commands.find(cmd);
  if (it != this->_commands.end())
    it->second->execute(shared_from_this(), args);
  else
    throw std::runtime_error("ko");
}
