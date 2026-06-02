#include "Game.hpp"
#include "Game/Player.hpp"
#include "Game/Team.hpp"

void game::GameLogic::lifeUpdate() {
  const std::vector<std::unique_ptr<Team>> &teams = game::GameLogic::getTeams();
  for (const auto &t : teams) {
    for (const auto &player : t->getPlayers()) {
      player->removeLife(1);
      if (player->isDead())
        std::cout << "dead" << std::endl;
    }
  }
}

void game::GameLogic::ressourcesUpdate() { return; }

void game::GameLogic::GameLoop() {
  // serverUpdate();
  // readCommands();
  // execCommands();
  lifeUpdate();
  ressourcesUpdate();
  // checkWinCond();
  // clearBuffers();
}
