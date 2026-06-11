#include "Game/Common.hpp"
#include "Game/Tile.hpp"
#include "GameLogic.hpp"

void game::GameLogic::playerForward(Player &player) {
  player.forward();
  player.setPos(((player.getX() % _mapX) + _mapX) % _mapX,
                ((player.getY() % _mapY) + _mapY) % _mapY);
  player.getClient()->sendMessage("ok\n");
}

void game::GameLogic::playerTurnRight(Player &player) {
  player.turnRight();
  player.getClient()->sendMessage("ok\n");
}

void game::GameLogic::playerTurnLeft(Player &player) {
  player.turnLeft();
  player.getClient()->sendMessage("ok\n");
}

void game::GameLogic::playerInventory(Player &player) {
  const auto &inv = player.getInventory();
  std::string msg = "[food " + std::to_string(inv[FOOD_IDX]) + ", linemate " +
                    std::to_string(inv[LINEMATE_IDX]) + ", deraumere " +
                    std::to_string(inv[DERAUMERE_IDX]) + ", sibur " +
                    std::to_string(inv[SIBUR_IDX]) + ", mendiane " +
                    std::to_string(inv[MENDIANE_IDX]) + ", phiras " +
                    std::to_string(inv[PHIRAS_IDX]) + ", thystame " +
                    std::to_string(inv[THYSTAME_IDX]) + "]\n";
  player.getClient()->sendMessage(msg);
}

void game::GameLogic::playerBroadcast(Player &player, const std::string &text) {
  for (const auto &team : _teams) {
    for (const auto &other : team->getPlayers()) {
      int dir = getDir(player, *other, _mapX, _mapY);
      other->getClient()->sendMessage("message " + std::to_string(dir) + ", " +
                                      text + "\n");
    }
  }
  player.getClient()->sendMessage("ok\n");
}

void game::GameLogic::playerConnectNbr(Player &player) {
  for (const auto &team : _teams) {
    for (const auto &other : team->getPlayers()) {
      if (other.get() == &player)
        player.getClient()->sendMessage(std::to_string(team->getAvailable()));
    }
  }
}

void game::GameLogic::playerEject(Player &player) {
  int x = player.getX();
  int y = player.getY();
  bool ejected = false;

  for (const auto &team : _teams) {
    for (const auto &other : team->getPlayers()) {
      if (other.get() == &player)
        continue;
      if (other->getX() == x && other->getY() == y) {
        int newX = x;
        int newY = y;
        switch (player.getOrientation()) {
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
        other->getClient()->sendMessage(
            "eject: " + std::to_string(player.getOrientation()) + "\n");
        ejected = true;
      }
    }
  }

  for (const auto &team : _teams) {
    std::vector<int> toRemove;
    for (const auto &egg : team->getEggs()) {
      if (egg->getX() == x && egg->getY() == y)
        toRemove.push_back(egg->getId());
    }
    for (int id : toRemove)
      team->removeEgg(id);
  }

  if (ejected)
    player.getClient()->sendMessage("ok\n");
  else
    player.getClient()->sendMessage("ko\n");
}

void game::GameLogic::playerTakeRessources(Player &player,
                                           std::string &toTake) {
  int index = getIndexByName(toTake);
  if (index == -1) {
    player.getClient()->sendMessage("ko\n");
    return;
  }
  Tile &tile = _map.getTile(player.getX(), player.getY());
  if (tile.getRessource(index) <= 0) {
    player.getClient()->sendMessage("ko\n");
    return;
  }
  tile.removeRessource(index, 1);
  player.addRessource(index, 1);
  player.getClient()->sendMessage("ok\n");
}

void game::GameLogic::playerDropRessources(Player &player,
                                           std::string &toDrop) {
  int index = getIndexByName(toDrop);
  if (index == -1) {
    player.getClient()->sendMessage("ko\n");
    return;
  }
  Tile &tile = _map.getTile(player.getX(), player.getY());
  if (player.getInventory()[index] <= 0) {
    player.getClient()->sendMessage("ko\n");
    return;
  }
  player.removeRessource(index, 1);
  tile.addRessource(index, 1);
  player.getClient()->sendMessage("ok\n");
}
