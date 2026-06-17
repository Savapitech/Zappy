#include "Client.hpp"
#include "Game/Common.hpp"
#include "Game/Player.hpp"
#include "Game/Tile.hpp"
#include "GameLogic.hpp"
#include "Server.hpp"

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
  player.getClient()->getServer().get().broadcastToGui(
      "pbc #" + std::to_string(player.getId()) + " " + text + "\n");
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

  if (ejected) {
    player.getClient()->sendMessage("ok\n");
    player.getClient()->getServer().get().broadcastToGui(
        "pex #" + std::to_string(player.getId()) + "\n");
  } else
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
  player.getClient()->getServer().get().broadcastToGui(
      "pgt #" + std::to_string(player.getId()) + " " + std::to_string(index) +
      "\n");
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
  player.getClient()->getServer().get().broadcastToGui(
      "pdr #" + std::to_string(player.getId()) + " " + std::to_string(index) +
      "\n");
}

void game::GameLogic::playerFork(Player &player) {
  int x = player.getX();
  int y = player.getY();
  int id = _nextId++;

  for (auto &team : _teams) {
    if (team->getName() == player.getTeamname()) {
      auto egg = std::make_unique<Egg>(id, x, y, player.getTeamname(), player.getId());
      team->addEgg(std::move(egg));
      team->addClientMax();
      break;
    }
  }
  player.getClient()->sendMessage("ok\n");
  auto &server = player.getClient()->getServer().get();
  server.broadcastToGui("pfk #" + std::to_string(player.getId()) + "\n");
  server.broadcastToGui("enw #" + std::to_string(id) + " #" +
                        std::to_string(player.getId()) + " " +
                        std::to_string(x) + " " + std::to_string(y) + "\n");
}

void game::GameLogic::playerLook(Player &player) {
  std::string to_send = "[";

  for (int dist = 0; dist <= player.getLevel(); dist++) {
        for (int side = -dist; side <= dist; side++) {
            int tx;
            int ty;
            switch (player.getOrientation()) {
                case N: 
                  tx = player.getX() + side;
                  ty = player.getY() - dist;
                  break;
                case E: 
                  tx = player.getX() + dist;
                  ty = player.getY() + side;
                  break;
                case S: 
                  tx = player.getX() - side;
                  ty = player.getY() + dist;
                  break;
                case W: 
                  tx = player.getX() - dist;
                  ty = player.getY() - side;
                  break;
                default:
                  tx = player.getX();
                  ty = player.getY();
            }
            
            tx = ((tx % _mapX) + _mapX) % _mapX;
            ty = ((ty % _mapY) + _mapY) % _mapY;

            Tile &tile = _map.getTile(tx, ty);

            for (const auto &team : _teams) {
                for (const auto &other : team->getPlayers()) {
                    if (other->getX() == tx && other->getY() == ty)
                        to_send += "player ";
                }
            }
            const std::string resNames[] = {
                "food", "linemate", "deraumere",
                "sibur", "mendiane", "phiras", "thystame"
            };
            for (int i = 0; i < RESOURCE_COUNT; i++) {
                for (int q = 0; q < tile.getRessource(i); q++)
                    to_send += resNames[i] + " ";
            }
            if (!to_send.empty() && to_send.back() == ' ')
                to_send.pop_back();
            to_send += ",";
        }
    }
    if (!to_send.empty() && to_send.back() == ',')
        to_send.pop_back();
    to_send += "]\n";
    player.getClient()->sendMessage(to_send);
}

void game::GameLogic::playerIncantation(Player &player) {
  int x = player.getX();
  int y = player.getY();
  int level = player.getLevel();

  static const int nbplayers[] = {1, 2, 2, 4, 4, 6, 6};
  static const int ressourcesNeeded[][RESOURCE_COUNT] = {
    {0, 1, 0, 0, 0, 0, 0},
    {0, 1, 1, 1, 0, 0, 0},
    {0, 2, 0, 1, 0, 2, 0},
    {0, 1, 1, 2, 0, 1, 0},
    {0, 1, 2, 1, 3, 0, 0},
    {0, 1, 2, 3, 0, 1, 0},
    {0, 2, 2, 2, 2, 2, 1},
  }; 

  if (level >= MAX_LVL) {
    player.getClient()->sendMessage("ko\n");
    return;
  }

  int idx = level - 1;
  Tile &tile = _map.getTile(x, y);

  for (int x = 0; x < RESOURCE_COUNT; x++) {
    if (tile.getRessource(x) < ressourcesNeeded[idx][x]) {
      player.getClient()->sendMessage("ko\n");
      return;
    }
  }

  std::vector<std::shared_ptr<Player>> incanting;
  for (auto &team : _teams) {
    for (auto &actual : team->getPlayers()) {
      if (actual->getX() == x && actual->getY() == y && actual->getLevel() == level)
        incanting.push_back(actual);
    }
  }

  if ((int)incanting.size() < nbplayers[idx]) {
      player.getClient()->sendMessage("ko\n");
      return;
  }

  auto &server = player.getClient()->getServer().get();
  std::string picMsg = "pic " + std::to_string(x) + " " + std::to_string(y) +
                       " " + std::to_string(level);
  for (auto &el : incanting)
    picMsg += " #" + std::to_string(el->getId());
  picMsg += "\n";
  server.broadcastToGui(picMsg);

  for (auto &el : incanting)
    el->setEvolving(true);

  for (int x = 0; x < RESOURCE_COUNT; x++) {
    tile.removeRessource(x, ressourcesNeeded[idx][x]);
  }

  for (auto &el : incanting) {
    el->levelup();
    el->setEvolving(false);
    el->getClient()->sendMessage("Current level: " + std::to_string(el->getLevel()) + "\n");
  }

  server.broadcastToGui("pie " + std::to_string(x) + " " + std::to_string(y) +
                        " ok\n");
}
