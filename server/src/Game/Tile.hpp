#pragma once

#include <array>

#include "Common.hpp"

namespace game {
class Tile {
private:
  std::array<int, RESOURCE_COUNT> _ressources;

public:
  Tile() { _ressources.fill(0); }
  void addRessource(int index, int qty) { _ressources[index] += qty; };
  void removeRessource(int index, int qty) { _ressources[index] -= qty; };
  int getRessource(int index) { return _ressources[index]; }
};
} // namespace game
