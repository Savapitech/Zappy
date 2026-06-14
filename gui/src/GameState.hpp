#pragma once
#include <map>
#include <string>
#include <vector>

namespace Zappy {

struct MapInfo {
  int width = 0;
  int height = 0;
  std::vector<std::string> teamNames;
  int timeUnit = 100;
  bool isInitialized = false;
};

struct Player {
  int id;
  int x, y;
  int orientation;
  int level;
  std::string team;
  int inventory[7] = {0};
};

struct Egg {
  int id;
  int playerId;
  int x;
  int y;
};

struct Tile {
  int resources[7] = {0};
};

struct GameState {
  MapInfo map;
  std::vector<Tile> grid;
  std::map<int, Player> players;
  std::map<int, Egg> eggs;
};

} // namespace Zappy