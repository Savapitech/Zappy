#include "NetworkManager.hpp"
#include <iostream>
#include <sstream>

#include "Logger.hpp"

namespace Zappy {

NetworkManager::NetworkManager(INetworkClient &client, std::string ip, int port)
    : _netClient(client), _ip(ip), _port(port) {
  initCommandHandlers();
}

NetworkManager::~NetworkManager() {}

void NetworkManager::initCommandHandlers() {
  _commandHandlers["msz"] = [this](const auto &args) { handleMsz(args); };
  _commandHandlers["bct"] = [this](const auto &args) { handleBct(args); };
  _commandHandlers["tna"] = [this](const auto &args) { handleTna(args); };
  _commandHandlers["sgt"] = [this](const auto &args) { handleSgt(args); };
  _commandHandlers["pnw"] = [this](const auto &args) { handlePnw(args); };
  _commandHandlers["ppo"] = [this](const auto &args) { handlePpo(args); };
  _commandHandlers["plv"] = [this](const auto &args) { handlePlv(args); };
  _commandHandlers["pin"] = [this](const auto &args) { handlePin(args); };
  _commandHandlers["pdi"] = [this](const auto &args) { handlePdi(args); };
  _commandHandlers["pbc"] = [this](const auto &args) { handlePbc(args); };
  _commandHandlers["pic"] = [this](const auto &args) { handlePic(args); };
  _commandHandlers["seg"] = [this](const auto &args) { handleSeg(args); };
  _commandHandlers["enw"] = [this](const auto &args) { handleEnw(args); };
  _commandHandlers["pgt"] = [this](const auto &args) { handlePgt(args); };
  _commandHandlers["pdr"] = [this](const auto &args) { handlePdr(args); };
  _commandHandlers["ebo"] = [this](const auto &args) { handleEbo(args); };
  _commandHandlers["edi"] = [this](const auto &args) { handleEdi(args); };
  _commandHandlers["pex"] = [this](const auto &args) { handlePex(args); };
  _commandHandlers["pie"] = [this](const auto &args) { handlePie(args); };
  _commandHandlers["pfk"] = [this](const auto &args) { handlePfk(args); };
  _commandHandlers["sst"] = [this](const auto &args) { handleSst(args); };
  _commandHandlers["smg"] = [this](const auto &args) { handleSmg(args); };
  _commandHandlers["suc"] = [this](const auto &args) { handleSuc(args); };
  _commandHandlers["sbp"] = [this](const auto &args) { handleSbp(args); };
}

bool NetworkManager::connectToServer() {
  return _netClient.connectToServer(_ip, _port);
}

bool NetworkManager::connectToServer(const std::string &host, int port) {
  return _netClient.connectToServer(host, port);
}
void NetworkManager::update() {
  std::vector<std::string> lines = _netClient.fetchLines();
  for (const std::string &line : lines) {
    processLine(line);
  }
}

void NetworkManager::sendCommand(const std::string &cmd) {
  _netClient.sendCommand(cmd);
}

std::vector<NetworkEvent> NetworkManager::consumeEvents() {
  auto events = std::move(_eventQueue);
  _eventQueue.clear();
  return events;
}

std::vector<std::string> NetworkManager::splitString(const std::string &str,
                                                     char delimiter) {
  std::vector<std::string> tokens;
  std::string token;
  std::istringstream tokenStream(str);
  while (std::getline(tokenStream, token, delimiter)) {
    if (!token.empty())
      tokens.push_back(token);
  }
  return tokens;
}

int NetworkManager::parseId(const std::string &idStr) {
  if (idStr.empty())
    return 0;
  return (idStr[0] == '#') ? std::stoi(idStr.substr(1)) : std::stoi(idStr);
}

void NetworkManager::processLine(const std::string &line) {
  if (line == "WELCOME") {
    handleWelcome();
    return;
  }

  auto args = splitString(line, ' ');
  if (args.empty())
    return;

  const std::string &cmd = args[0];

  auto it = _commandHandlers.find(cmd);
  if (it != _commandHandlers.end()) {
    it->second(args);
  } else {
    LOG_WARN("UNKNOWN_COMMAND:" + line);
  }
}

void NetworkManager::handleWelcome() {
  sendCommand("GRAPHIC\n");
  sendCommand("msz\n");
  sendCommand("tna\n");
  sendCommand("sgt\n");
}

void NetworkManager::handleMsz(const std::vector<std::string> &args) {
  if (args.size() >= 3) {
    _gameState.map.width = std::stoi(args[1]);
    _gameState.map.height = std::stoi(args[2]);
    _gameState.grid.resize(_gameState.map.width * _gameState.map.height);
    _gameState.map.isInitialized = true;

    sendCommand("mct\n");
  }
}

void NetworkManager::handleBct(const std::vector<std::string> &args) {
  if (args.size() >= 10 && _gameState.map.isInitialized) {
    int x = std::stoi(args[1]);
    int y = std::stoi(args[2]);
    int index = y * _gameState.map.width + x;
    if (index >= 0 && static_cast<size_t>(index) < _gameState.grid.size()) {
      for (int i = 0; i < 7; ++i) {
        _gameState.grid[index].resources[i] = std::stoi(args[3 + i]);
      }
      _eventQueue.push_back({NetworkEventType::TILE_UPDATED, args});
    }
  }
}

void NetworkManager::handleTna(const std::vector<std::string> &args) {
  if (args.size() >= 2) {
    _gameState.map.teamNames.push_back(args[1]);
  }
}

void NetworkManager::handleSgt(const std::vector<std::string> &args) {
  if (args.size() >= 2) {
    _gameState.map.timeUnit = std::stoi(args[1]);
  }
}

void NetworkManager::handlePnw(const std::vector<std::string> &args) {
  if (args.size() >= 7) {
    int id = parseId(args[1]);
    Player p;
    p.id = id;
    p.x = std::stoi(args[2]);
    p.y = std::stoi(args[3]);
    p.orientation = std::stoi(args[4]);
    p.level = std::stoi(args[5]);
    p.team = args[6];

    _gameState.players[id] = p;
    _eventQueue.push_back({NetworkEventType::PLAYER_CONNECTED, args});
  }
}

void NetworkManager::handlePpo(const std::vector<std::string> &args) {
  if (args.size() >= 5) {
    int id = parseId(args[1]);
    if (_gameState.players.contains(id)) {
      _gameState.players[id].x = std::stoi(args[2]);
      _gameState.players[id].y = std::stoi(args[3]);
      _gameState.players[id].orientation = std::stoi(args[4]);

      _eventQueue.push_back({NetworkEventType::PLAYER_MOVED, args});
    }
  }
}

void NetworkManager::handlePlv(const std::vector<std::string> &args) {
  if (args.size() >= 3) {
    int id = parseId(args[1]);
    if (_gameState.players.contains(id)) {
      _gameState.players[id].level = std::stoi(args[2]);
      _eventQueue.push_back({NetworkEventType::PLAYER_LEVEL_UP, args});
    }
  }
}

void NetworkManager::handlePin(const std::vector<std::string> &args) {
  if (args.size() >= 11) {
    int id = parseId(args[1]);
    if (_gameState.players.contains(id)) {
      for (int i = 0; i < 7; ++i) {
        _gameState.players[id].inventory[i] = std::stoi(args[4 + i]);
      }
    }
  }
}

void NetworkManager::handlePdi(const std::vector<std::string> &args) {
  if (args.size() >= 2) {
    int id = parseId(args[1]);
    _gameState.players.erase(id);
    _eventQueue.push_back({NetworkEventType::PLAYER_DISCONNECTED, args});
  }
}

void NetworkManager::handlePbc(const std::vector<std::string> &args) {
  _eventQueue.push_back({NetworkEventType::BROADCAST, args});
}

void NetworkManager::handlePic(const std::vector<std::string> &args) {
  _eventQueue.push_back({NetworkEventType::INCANTATION_START, args});
}

void NetworkManager::handleSeg(const std::vector<std::string> &args) {
  _eventQueue.push_back({NetworkEventType::GAME_OVER, args});
}

void NetworkManager::handleEnw(const std::vector<std::string> &args) {
  if (args.size() >= 5) {
    _eventQueue.push_back({NetworkEventType::EGG_LAID, args});
  }
}

void NetworkManager::handlePgt(const std::vector<std::string> &args) {
  if (args.size() >= 3) {
    _eventQueue.push_back({NetworkEventType::RESOURCE_COLLECTED, args});
    std::string idStr = args[1];

    if (!idStr.empty() && idStr[0] == '#') {
      idStr = idStr.substr(1);
    }
    this->sendCommand("pin " + idStr + "\n");
  }
}

void NetworkManager::handlePdr(const std::vector<std::string> &args) {
  if (args.size() >= 3) {
    _eventQueue.push_back({NetworkEventType::RESOURCE_DROPPED, args});
    std::string idStr = args[1];

    if (!idStr.empty() && idStr[0] == '#') {
      idStr = idStr.substr(1);
    }
    this->sendCommand("pin " + idStr + "\n");
  }
}

void NetworkManager::handleEbo(const std::vector<std::string> &args) {
  if (args.size() >= 2) {
    _eventQueue.push_back({NetworkEventType::EGG_HATCHED, args});
  }
}

void NetworkManager::handleEdi(const std::vector<std::string> &args) {
  if (args.size() >= 2) {
    _eventQueue.push_back({NetworkEventType::EGG_DIED, args});
  }
}

void NetworkManager::handlePex(const std::vector<std::string> &args) {
  if (args.size() >= 2) {
    _eventQueue.push_back({NetworkEventType::PLAYER_EXPULSED, args});
  }
}

void NetworkManager::handlePie(const std::vector<std::string> &args) {
  if (args.size() >= 4) {
    _eventQueue.push_back({NetworkEventType::INCANTATION_END, args});
  }
}

void NetworkManager::handlePfk(const std::vector<std::string> &args) {
  if (args.size() >= 2) {
    _eventQueue.push_back({NetworkEventType::EGG_LAYING, args});
  }
}

void NetworkManager::handleSst(const std::vector<std::string> &args) {
  if (args.size() >= 2) {
    _gameState.map.timeUnit = std::stoi(args[1]);
    _eventQueue.push_back({NetworkEventType::TIME_UNIT_MODIFIED, args});
  }
}

void NetworkManager::handleSmg(const std::vector<std::string> &args) {
  if (args.size() >= 2) {
    _eventQueue.push_back({NetworkEventType::SERVER_MESSAGE, args});
  }
}

void NetworkManager::handleSuc(const std::vector<std::string> &args) {
  LOG_ERROR("Server reported: Unknown command (suc)");
  _eventQueue.push_back({NetworkEventType::SERVER_ERROR, args});
}

void NetworkManager::handleSbp(const std::vector<std::string> &args) {
  LOG_ERROR("Server reported: Bad command parameter (sbp)");
  _eventQueue.push_back({NetworkEventType::SERVER_ERROR, args});
}

void NetworkManager::sendSst(int time) {
  std::string format = "sst " + std::to_string(time) + "\n";
  this->sendCommand(format);
}

void NetworkManager::sendSgt() { this->sendCommand(std::string("sgt\n")); }

} // namespace Zappy
