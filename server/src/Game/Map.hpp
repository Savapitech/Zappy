#pragma once

#include <vector>

#include "Tile.hpp"

namespace game {
class Map {
private:
  int _width;
  int _heigth;
  std::vector<Tile> _tiles;

public:
  Map(int width, int heigth)
      : _width(width), _heigth(heigth), _tiles(width * heigth) {}
  int getWidth() const { return _width; }
  int getHeigth() const { return _heigth; }
  Tile &getTile(int x, int y) {
    int newX = ((x % _width) + _width) % _width;
    int newY = ((y % _heigth) + _heigth) % _heigth;
    return _tiles[newY + _width + newX];
  }
};
} // namespace game
