#include "Player.hpp"

void game::Player::setPos(int x, int y) {
  _x = x;
  _y = y;
}

void game::Player::removeLife(int life) {
  _lifeUnits -= life;
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
