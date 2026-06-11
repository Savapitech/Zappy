#include <chrono>

#include "Client.hpp"
#include "Common.hpp"
#include "Game/Player.hpp"
#include "GameLogic.hpp"
#include "Team.hpp"

//---------------------Init functions-----------------------

void game::GameLogic::initRessources() {
  int nbTiles = _mapX * _mapY;
  int ressources[RESOURCE_COUNT] = {
      (int)(nbTiles * FOOD_DENSITY),      (int)(nbTiles * LINEMATE_DENSITY),
      (int)(nbTiles * DERAUMERE_DENSITY), (int)(nbTiles * SIBUR_DENSITY),
      (int)(nbTiles * MENDIANE_DENSITY),  (int)(nbTiles * PHIRAS_DENSITY),
      (int)(nbTiles * THYSTAME_DENSITY),
  };
  for (int i = 0; i < RESOURCE_COUNT; i++)
    if (ressources[i] < 1)
      ressources[i]++;

  srand(time(NULL));
  for (int z = 0; z < RESOURCE_COUNT; z++) {
    for (int a = 0; a < ressources[z]; a++) {
      int x = rand() % _mapX;
      int y = rand() % _mapY;
      _map.getTile(x, y).addRessource(z, 1);
    }
  }
}

void game::GameLogic::initTeams(std::vector<std::string> teamnames) {
  for (const auto &name : teamnames) {
    auto team = std::make_unique<Team>(name, _nbClientMax);
    _teams.push_back(std::move(team));
  }
}

void game::GameLogic::initEggs() {
  for (const auto &team : _teams) {
    if (team->getName() == GUI_TEAM)
      continue;
    for (int i = 0; i < _nbClientMax; i++) {
      int x = rand() % _mapX;
      int y = rand() % _mapY;
      int id = _nextId++;
      auto egg = std::make_unique<Egg>(id, x, y, team->getName(), -1);
      team->addEgg(std::move(egg));
      team->addClientMax();
    }
  }
}

game::GameLogic::GameLogic(int x, int y, int freq, int nbClientMax,
                           std::vector<std::string> teamnames)
    : _mapX(x), _mapY(y), _freq(freq), _nbClientMax(nbClientMax), _nextId(0),
      _map(x, y) {
  _lastLifeTime = std::chrono::steady_clock::now();
  _lastRessourceTime = std::chrono::steady_clock::now();
  initRessources();
  initTeams(teamnames);
  initEggs();
}

//---------------GameLoop functions-----------------------

void game::GameLogic::ressourcesUpdate() {
  auto now = std::chrono::steady_clock::now();
  double intervalMax = (double)RESOURCE_RESPAWN / _freq;
  auto timeElapsed =
      std::chrono::duration<double>(now - _lastRessourceTime).count();
  if (intervalMax > timeElapsed)
    return;

  int nbTiles = _mapX * _mapY;
  int actual[RESOURCE_COUNT] = {0};
  for (int y = 0; y < _mapY; y++) {
    for (int x = 0; x < _mapX; x++) {
      for (int i = 0; i < RESOURCE_COUNT; i++) {
        actual[i] += _map.getTile(x, y).getRessource(i);
      }
    }
  }

  int ressources[RESOURCE_COUNT] = {
      (int)(nbTiles * FOOD_DENSITY),      (int)(nbTiles * LINEMATE_DENSITY),
      (int)(nbTiles * DERAUMERE_DENSITY), (int)(nbTiles * SIBUR_DENSITY),
      (int)(nbTiles * MENDIANE_DENSITY),  (int)(nbTiles * PHIRAS_DENSITY),
      (int)(nbTiles * THYSTAME_DENSITY),
  };

  for (int index = 0; index < RESOURCE_COUNT; index++) {
    int missing = ressources[index] - actual[index];
    if (missing > 0) {
      for (int b = 0; b < missing; b++) {
        int x = rand() % _mapX;
        int y = rand() % _mapY;
        _map.getTile(x, y).addRessource(index, 1);
      }
    }
  }
}

void game::GameLogic::lifeUpdate() {
  auto now = std::chrono::steady_clock::now();
  double intervalMax = (double)SURVIVAL_TIME / _freq;
  auto timeElapsed = std::chrono::duration<double>(now - _lastLifeTime).count();
  if (intervalMax > timeElapsed)
    return;

  const std::vector<std::unique_ptr<Team>> &teams = game::GameLogic::getTeams();
  for (const auto &t : teams) {
    for (const auto &player : t->getPlayers()) {
      player->removeLife(1);
      if (player->isDead())
        player->getClient()->sendMessage("is dead\n");
    }
  }
}

bool game::GameLogic::checkWinCond() {
  const std::vector<std::unique_ptr<Team>> &teams = game::GameLogic::getTeams();
  int count = 0;
  for (const auto &t : teams) {
    for (const auto &p : t->getPlayers()) {
      if (p->getLevel() == MAX_LVL)
        count++;
    }
    if (count >= WIN_COND) {
      for (const auto &t : teams) {
        for (const auto &p : t->getPlayers()) {
          p->getClient()->sendMessage(t->getName() + "a gagné la partie\n");
        }
      }
      return true;
    }
  }
  return false;
}

void game::GameLogic::newPlayer(Client &client, const std::string &teamname) {
  if (teamname == GUI_TEAM)
    return; // need to implement this later

  for (auto &team : _teams) {
    if (team->getName() != teamname)
      continue;
    if (team->getAvailable() < 1) {
      client.sendMessage("ko\n");
      return;
    }
    int id = getNextId();
    auto player = std::make_shared<Player>(id, teamname);
    auto egg = team->pickRandomEgg();
    if (!egg) {
      client.sendMessage("ko\n");
      return;
    }
    player->setPos(egg->get().getX(), egg->get().getY());
    player->setOrientation(rand() % 4 + 1);
    team->removeEgg(egg->get().getId());
    team->addConnected();
    client.setPlayer(player);
    client.sendMessage(std::to_string(team->getAvailable()) + "\n");
    client.sendMessage(std::to_string(getMapX()) + " " +
                       std::to_string(getMapY()) + "\n");
  }
  client.sendMessage("ko\n");
}

//--------------------Utils functions-----------------------

void game::GameLogic::Debug() { return; }

void game::GameLogic::poll() {
  lifeUpdate();
  ressourcesUpdate();
  checkWinCond();
  Debug();
}

int game::GameLogic::getDir(Player &player, Player &other, int width,
                            int heigth) {
  int disY = other.getY() - player.getY();
  int disX = other.getX() - player.getX();

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

  switch (player.getOrientation()) {
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

int game::GameLogic::getIndexByName(std::string &toTake) {
  if (toTake == "food")
    return FOOD_IDX;
  if (toTake == "linemate")
    return LINEMATE_IDX;
  if (toTake == "deraumere")
    return DERAUMERE_IDX;
  if (toTake == "sibur")
    return SIBUR_IDX;
  if (toTake == "mendiane")
    return MENDIANE_IDX;
  if (toTake == "phiras")
    return PHIRAS_IDX;
  if (toTake == "thystame")
    return THYSTAME_IDX;
  return -1;
}
