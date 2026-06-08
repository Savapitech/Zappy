#include "Game.hpp"

void game::GameLogic::playerForward(Player *player) {
  player->forward();
  player->setPos(((player->getX() % _mapX) + _mapX) % _mapX,
                 ((player->getY() % _mapY) + _mapY) % _mapY);
  player->getClient().sendMessage("ok\n");
}

void game::GameLogic::playerTurnRight(Player *player) {
  player->turnRight();
  player->getClient().sendMessage("ok\n");
}

void game::GameLogic::playerTurnLeft(Player *player) {
  player->turnLeft();
  player->getClient().sendMessage("ok\n");
}

void game::GameLogic::playerInventory(Player *player) {
  const auto &inv = player->getInventory();
  std::string msg = "[food " + std::to_string(inv[FOOD_IDX]) + ", linemate " +
                    std::to_string(inv[LINEMATE_IDX]) + ", deraumere " +
                    std::to_string(inv[DERAUMERE_IDX]) + ", sibur " +
                    std::to_string(inv[SIBUR_IDX]) + ", mendiane " +
                    std::to_string(inv[MENDIANE_IDX]) + ", phiras " +
                    std::to_string(inv[PHIRAS_IDX]) + ", thystame " +
                    std::to_string(inv[THYSTAME_IDX]) + "]\n";
  player->getClient().sendMessage(msg);
}

static int getDir(game::Player *player, game::Player *other, int width,
                  int heigth) {
  int disY = other->getY() - player->getY();
  int disX = other->getX() - player->getX();

  if (disY == 0 && disX == 0)
    return 0;

  if (disX > width / 2)
    disX -= width;
  if (disY > heigth / 2)
    disY -= heigth;
  if (disX < -width / 2)
    disX += width;
  if (disY < -heigth / 2)
    disY += heigth;

  switch (player->getOrientation()) {
  case N:
    if (disY < 0 && disX == 0)
      return 1;
    if (disY < 0 && disX > 0)
      return 2;
    if (disY == 0 && disX > 0)
      return 3;
    if (disY > 0 && disX > 0)
      return 4;
    if (disY > 0 && disX == 0)
      return 5;
    if (disY > 0 && disX < 0)
      return 6;
    if (disY == 0 && disX < 0)
      return 7;
    if (disY < 0 && disX < 0)
      return 8;
    break;
  case E:
    if (disX > 0 && disY == 0)
      return 1;
    if (disX > 0 && disY > 0)
      return 2;
    if (disX == 0 && disY > 0)
      return 3;
    if (disX < 0 && disY > 0)
      return 4;
    if (disX < 0 && disY == 0)
      return 5;
    if (disX < 0 && disY < 0)
      return 6;
    if (disX == 0 && disY < 0)
      return 7;
    if (disX > 0 && disY < 0)
      return 8;
    break;
  case S:
    if (disY > 0 && disX == 0)
      return 1;
    if (disY > 0 && disX < 0)
      return 2;
    if (disY == 0 && disX < 0)
      return 3;
    if (disY < 0 && disX < 0)
      return 4;
    if (disY < 0 && disX == 0)
      return 5;
    if (disY < 0 && disX > 0)
      return 6;
    if (disY == 0 && disX > 0)
      return 7;
    if (disY > 0 && disX > 0)
      return 8;
    break;
  case W:
    if (disX < 0 && disY == 0)
      return 1;
    if (disX < 0 && disY < 0)
      return 2;
    if (disX == 0 && disY < 0)
      return 3;
    if (disX > 0 && disY < 0)
      return 4;
    if (disX > 0 && disY == 0)
      return 5;
    if (disX > 0 && disY > 0)
      return 6;
    if (disX == 0 && disY > 0)
      return 7;
    if (disX < 0 && disY > 0)
      return 8;
    break;
  }
  return 0;
}

void game::GameLogic::playerBroadcast(Player *player, const std::string &text) {
  for (const auto &team : _teams) {
    for (const auto &other : team->getPlayers()) {
      int dir = getDir(player, other.get(), _mapX, _mapY);
      other->getClient().sendMessage("message " + std::to_string(dir) + ", " +
                                     text + "\n");
    }
  }
  player->getClient().sendMessage("ok\n");
}

void game::GameLogic::playerEject(Player *player) {
  int x = player->getX();
  int y = player->getY();
  bool ejected = false;

  for (const auto &team : _teams) {
    for (const auto &other : team->getPlayers()) {
      if (other.get() == player)
        continue;
      if (other->getX() == x && other->getY() == y) {
        int newX = x;
        int newY = y;
        switch (player->getOrientation()) {
        case N:
          newY--;
          break;
        case E:
          newX++;
          break;
        case S:
          newY++;
          break;
        case W:
          newX--;
          break;
        }

        newX = ((newX % _mapX) + _mapX) % _mapX;
        newY = ((newY % _mapY) + _mapY) % _mapY;
        other->setPos(newX, newY);
        other->getClient().sendMessage(
            "eject: " + std::to_string(player->getOrientation()) + "\n");
        ejected = true;
      }
    }
  }
  if (ejected)
    player->getClient().sendMessage("ok\n");
  else
    player->getClient().sendMessage("ko\n");
}
