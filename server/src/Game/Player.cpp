#include "Player.hpp"
#include "Game/Common.hpp"

void game::Player::setPos(int x, int y) {
  _x = x;
  _y = y;
}

void game::Player::removeLife(int life) {
  _lifeUnits -= life;
  while (_lifeUnits <= 0 && _inventory[FOOD_IDX] > 0) {
    _inventory[FOOD_IDX]--;
    _lifeUnits += SURVIVAL_TIME;
  }
  if (_lifeUnits <= 0) {
    _lifeUnits = 0;
    _isDead = true;
  }
}

void game::Player::removeRessource(int index, int qty) {
  _inventory[index] -= qty;
}

void game::Player::addRessource(int index, int qty) {
  _inventory[index] += qty;
}

int game::Player::forward() {
  // check time for action 7/f
  if (_Orientation == N)
    _y--;
  if (_Orientation == E)
    _x++;
  if (_Orientation == S)
    _y++;
  if (_Orientation == W)
    _x--;
  return 0;
}

int game::Player::turnRight() {
  // check time for action 7/f
  _Orientation++;
  if (_Orientation > W)
    _Orientation = N;
  return 0;
}

int game::Player::turnLeft() {
  // check time for action 7/f
  _Orientation--;
  if (_Orientation < N)
    _Orientation = W;
  return 0;
}

game::Player::Player(int id, const std::string &teamname)
    : _id(id), _Orientation(N), _lifeUnits(START_LIFE), _level(1),
      _teamname(teamname), _isDead(false), _isEvolving(false), _inventory{} {}
