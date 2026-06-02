#pragma once

#include <memory>
#include <vector>

#include "Common.hpp"
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

public:
  GameLogic(int x, int y, int freq, int nbClientMax);

  const std::vector<std::unique_ptr<Team>> &getTeams() const { return _teams; }
  int getMapX() const { return _mapX; }
  int getMapY() const { return _mapY; }
  int getFreq() const { return _freq; }
  int getClientMax() const { return _nbClientMax; }
  int getNextId() const { return _nextId; }

  void GameLoop();
  void lifeUpdate();
  void ressourcesUpdate();
  void checkWinCond();
};
} // namespace game
