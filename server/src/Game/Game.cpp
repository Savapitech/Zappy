#include "Game.hpp"
#include "Common.hpp"
#include "Map.hpp"

#include <chrono>

void game::GameLogic::initRessources() {
  int nbTiles = _mapX * _mapY;
  int ressources[RESOURCE_COUNT] = {
    (int)(nbTiles * FOOD_DENSITY),
    (int)(nbTiles * LINEMATE_DENSITY),
    (int)(nbTiles * DERAUMERE_DENSITY),
    (int)(nbTiles * SIBUR_DENSITY),
    (int)(nbTiles * MENDIANE_DENSITY),
    (int)(nbTiles * PHIRAS_DENSITY),
    (int)(nbTiles * THYSTAME_DENSITY),
  };
  for (int i = 0; i < RESOURCE_COUNT; i++)
    if (ressources[i] < 1)
      ressources[i]++;

  srand(time(NULL));
  for (int z = 0; z < RESOURCE_COUNT; z++) {
    for (int a = 0; a < ressources[z]; a++){
      int x = rand() % _mapX;
      int y = rand() % _mapY;
      _map.getTile(x, y).addRessource(z, 1);
    }
  }
}

void game::GameLogic::ressourcesUpdate() {
  auto now = std::chrono::steady_clock::now();
  double intervalMax = (double)RESOURCE_RESPAWN / _freq;
  auto timeElapsed = std::chrono::duration<double>(now - _lastRessourceTime).count();
  if (intervalMax > timeElapsed)
    return;

  int nbTiles = _mapX * _mapY;
  int actual[RESOURCE_COUNT] = {0};
  for (int y = 0; y < _mapY; y++) {
    for (int x = 0; x < _mapX; x++) {
      for (int i = 0; i < RESOURCE_COUNT; i++) {
        actual[i] += _map.getTile(x, y).getRessource(i);
      }
    }
  }

  int ressources[RESOURCE_COUNT] = {
    (int)(nbTiles * FOOD_DENSITY),
    (int)(nbTiles * LINEMATE_DENSITY),
    (int)(nbTiles * DERAUMERE_DENSITY),
    (int)(nbTiles * SIBUR_DENSITY),
    (int)(nbTiles * MENDIANE_DENSITY),
    (int)(nbTiles * PHIRAS_DENSITY),
    (int)(nbTiles * THYSTAME_DENSITY),
  };

  for (int index = 0; index < RESOURCE_COUNT; index++) {
    int missing = ressources[index] - actual[index];
    if (missing > 0) {
      for (int b = 0; b < missing; b++) {
        int x = rand() % _mapX;
        int y = rand() % _mapY;
        _map.getTile(x, y).addRessource(index, 1);
      }
    }
  }
}

void game::GameLogic::lifeUpdate() {
  auto now = std::chrono::steady_clock::now();
  double intervalMax = (double)SURVIVAL_TIME / _freq;
  auto timeElapsed = std::chrono::duration<double>(now - _lastLifeTime).count();
  if (intervalMax > timeElapsed)
    return;

  const std::vector<std::unique_ptr<Team>> &teams = game::GameLogic::getTeams();
  for (const auto &t : teams) {
    for (const auto &player : t->getPlayers()) {
      player->removeLife(1);
      if (player->isDead())
        std::cout << "dead" << std::endl;
    }
  }
}

game::GameLogic::GameLogic(int x, int y, int freq, int nbClientMax)
  : _mapX(x), _mapY(y), _freq(freq), _nbClientMax(nbClientMax), _nextId(0), _map(x, y) { 
    _lastLifeTime = std::chrono::steady_clock::now();
    _lastRessourceTime = std::chrono::steady_clock::now();
    initRessources();
  }
