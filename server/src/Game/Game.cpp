#include "Game.hpp"
#include "Common.hpp"
#include "Map.hpp"

void initRessources(game::Map &map, int width, int heigth) {
  int nbTiles = width * heigth;
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
      int x = rand() % width;
      int y = rand() % heigth;
      map.getTile(x, y).addRessource(z, 1);
    }
  }
}

game::GameLogic::GameLogic(int x, int y, int freq, int nbClientMax)
  : _mapX(x), _mapY(y), _freq(freq), _nbClientMax(nbClientMax), _nextId(0), _map(x, y) {
    initRessources(_map, _mapX, _mapY);
  }
