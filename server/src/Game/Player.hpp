#pragma once

#include <array>
#include <string>

#include "Client.hpp"
#include "Common.hpp"

namespace game {
class Player {
private:
  int _id;
  int _x;
  int _y;
  int _Orientation;
  int _lifeUnits;
  int _level;
  std::string _teamname;
  bool _isDead;
  bool _isEvolving;
  std::array<int, RESOURCE_COUNT> _inventory;
  Client _client;

public:
  int getId() const { return _id; }
  int getLife() const { return _lifeUnits; }
  int getLevel() const { return _level; }
  int getX() const { return _x; }
  int getY() const { return _y; }
  int getOrientation() const { return _Orientation; }
  const std::string &getTeamname() { return _teamname; }
  bool isDead() const { return _isDead; }
  bool isEvolving() const { return _isEvolving; }
  int getRessource(int index) { return _inventory[index]; }
  const std::array<int, RESOURCE_COUNT> &getInventory() { return _inventory; }
  Client &getClient() { return _client; }

  void setPos(int x, int y);
  void setOrientation(int orientation) { _Orientation = orientation; }
  void addLife(int life) { _lifeUnits += life; }
  void removeLife(int life);
  void levelup() { _level++; }
  void setEvolving(bool evolve) { _isEvolving = evolve; }
  void addRessource(int index, int qty);
  void removeRessource(int index, int qty);

  int forward();
  int turnRight();
  int turnLeft();
  int Look();
  int inventory();
  int broadcastText();
  int connectNbr();
  int fork();
  int eject();
  int takeObj();
  int dropObj();
  int startIncentation();

};
} // namespace game
