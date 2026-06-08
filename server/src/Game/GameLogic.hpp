#pragma once

#include <memory>
#include <vector>

#include "Game/Player.hpp"
#include "Map.hpp"
#include "Team.hpp"

namespace game {
class GameLogic {
private:
  std::vector<std::unique_ptr<Team>> _teams;
  int _mapX;
  int _mapY;
  int _freq;
  int _nbClientMax;
  int _nextId;
  Map _map;
  std::chrono::steady_clock::time_point _lastLifeTime;
  std::chrono::steady_clock::time_point _lastRessourceTime;

public:
  // getter and constructor
  GameLogic(int x, int y, int freq, int nbClientMax);
  const std::vector<std::unique_ptr<Team>> &getTeams() const { return _teams; }
  int getMapX() const { return _mapX; }
  int getMapY() const { return _mapY; }
  int getFreq() const { return _freq; }
  int getClientMax() const { return _nbClientMax; }
  int getNextId() const { return _nextId; }

  // main loop
  void initRessources();
  void GameLoop();
  void lifeUpdate();
  void ressourcesUpdate();
  bool checkWinCond();

  // commands
  void playerForward(Player *player);
  void playerTurnRight(Player *player);
  void playerTurnLeft(Player *player);
  void playerInventory(Player *player);
  void playerBroadcast(Player *player, const std::string &text);
  void playerConnectNbr(Player *player);
  void playerEject(Player *player);
  void playerTakeRessources(Player *player, std::string &toTake);
  void playerDropRessources(Player *player, std::string &toDrop);

  // utils
  int getDir(game::Player *player, game::Player *other, int width, int heigth);
  int getIndexByName(std::string &toTake);
};
} // namespace game
