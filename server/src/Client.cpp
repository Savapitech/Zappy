#include <arpa/inet.h>
#include <cerrno>
#include <stdexcept>
#include <unistd.h>

#include "Client.hpp"
#include "Commands/Broadcast.hpp"
#include "Commands/ConnectNbr.hpp"
#include "Commands/Eject.hpp"
#include "Commands/Fork.hpp"
#include "Commands/Forward.hpp"
#include "Commands/Gui/Bct.hpp"
#include "Commands/Gui/Mct.hpp"
#include "Commands/Gui/Msz.hpp"
#include "Commands/Gui/Pin.hpp"
#include "Commands/Gui/Plv.hpp"
#include "Commands/Gui/Ppo.hpp"
#include "Commands/Gui/Sgt.hpp"
#include "Commands/Gui/Sst.hpp"
#include "Commands/Gui/Tna.hpp"
#include "Commands/Incantation.hpp"
#include "Commands/Inventory.hpp"
#include "Commands/Left.hpp"
#include "Commands/Look.hpp"
#include "Commands/Right.hpp"
#include "Commands/Set.hpp"
#include "Commands/Take.hpp"
#include "Game/Common.hpp"
#include "Game/GameLogic.hpp"
#include "Logger.hpp"
#include "Parser.hpp"
#include "Server.hpp"

void Client::registerCommands() {
  this->_aiCommands["Forward"] = std::make_shared<commands::Forward>();
  this->_aiCommands["Right"] = std::make_shared<commands::Right>();
  this->_aiCommands["Left"] = std::make_shared<commands::Left>();
  this->_aiCommands["Inventory"] = std::make_shared<commands::Inventory>();
  this->_aiCommands["Broadcast"] = std::make_shared<commands::Broadcast>();
  this->_aiCommands["Connect_nbr"] = std::make_shared<commands::ConnectNbr>();
  this->_aiCommands["Eject"] = std::make_shared<commands::Eject>();
  this->_aiCommands["Take"] = std::make_shared<commands::Take>();
  this->_aiCommands["Set"] = std::make_shared<commands::Set>();
  this->_aiCommands["Fork"] = std::make_shared<commands::Fork>();
  this->_aiCommands["Look"] = std::make_shared<commands::Look>();
  this->_aiCommands["Incantation"] = std::make_shared<commands::Incantation>();

  this->_guiCommands["msz"] = std::make_shared<commands::gui::Msz>();
  this->_guiCommands["bct"] = std::make_shared<commands::gui::Bct>();
  this->_guiCommands["mct"] = std::make_shared<commands::gui::Mct>();
  this->_guiCommands["tna"] = std::make_shared<commands::gui::Tna>();
  this->_guiCommands["ppo"] = std::make_shared<commands::gui::Ppo>();
  this->_guiCommands["plv"] = std::make_shared<commands::gui::Plv>();
  this->_guiCommands["pin"] = std::make_shared<commands::gui::Pin>();
  this->_guiCommands["sgt"] = std::make_shared<commands::gui::Sgt>();
  this->_guiCommands["sst"] = std::make_shared<commands::gui::Sst>();
}

Client::Client(int fd, sockaddr_in addr, std::reference_wrapper<Server> server)
    : _fd(fd), _addr(addr), _server(server) {
  registerCommands();
  sendMessage("WELCOME\n");
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
  if (this->_fd < 0 || msg.empty())
    return;
  this->_writeBuffer += msg;
  LOG_DEBUG(std::format("Message queued for client [{}]",
                        msg.substr(0, msg.size() - 1)));
}

void Client::flushWrite() {
  if (this->_fd < 0 || this->_writeBuffer.empty())
    return;

  ssize_t written =
      ::write(this->_fd, this->_writeBuffer.data(), this->_writeBuffer.size());
  if (written > 0)
    this->_writeBuffer.erase(0, written);
  else if (written < 0 && errno != EAGAIN && errno != EWOULDBLOCK)
    this->_isConnected = false;
}

void Client::handleMessage() {
  char buffer[4096];
  ssize_t bytesRead = ::read(this->_fd, buffer, sizeof(buffer) - 1);

  if (bytesRead == 0) {
    this->_isConnected = false;
    return;
  }
  if (bytesRead < 0) {
    if (errno != EAGAIN && errno != EWOULDBLOCK)
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

  if (!this->_handshakeDone) {
    try {
      this->_server.get().getGame().newPlayer(*this, commandLine);
    } catch (std::exception &e) {
      LOG_ERROR(e.what());
      this->disconnect();
      return;
    }
    this->_handshakeDone = true;
    return;
  }

  std::string cmd = commandLine.substr(0, commandLine.find_first_of(" \t"));
  std::vector<std::string> args = ParseArgs(commandLine.substr(cmd.size()));

  LOG_DEBUG(
      std::format("Parsed command name [{}] args size [{}]", cmd, args.size()));

  auto &commands =
      (this->_type == ClientType::GUI) ? this->_guiCommands : this->_aiCommands;

  auto it = commands.find(cmd);
  if (it == commands.end()) {
    this->sendMessage(_type == ClientType::GUI ? "suc\n" : "ko\n");
    throw std::runtime_error(std::format("Unknown command [{}]", commandLine));
  }

  if (this->_type == ClientType::GUI) {
    it->second->execute(shared_from_this(), args);
    return;
  }

  if (this->_player && this->_player->isEvolving()) {
    this->sendMessage("ko\n");
    return;
  }

  if (this->_pendingCommands.size() >= MAX_CMD_QUEUE)
    return;
  this->_pendingCommands.push_back({it->second, args});
}

void Client::tick(int freq) {
  if (this->_type != ClientType::AI)
    return;

  auto now = std::chrono::steady_clock::now();

  if (this->_executingCommand) {
    if (now < this->_commandReadyAt)
      return;
    this->_executingCommand = false;
    PendingCommand pending = std::move(this->_pendingCommands.front());
    this->_pendingCommands.pop_front();
    pending.command->execute(shared_from_this(), pending.args);
    now = std::chrono::steady_clock::now();
  }

  if (this->_pendingCommands.empty())
    return;

  auto &next = this->_pendingCommands.front();
  next.command->onQueued(shared_from_this());
  double delay = freq > 0 ? (double)next.command->getCost() / freq : 0.0;
  this->_commandReadyAt =
      now + std::chrono::duration_cast<std::chrono::steady_clock::duration>(
                std::chrono::duration<double>(delay));
  this->_executingCommand = true;
}
