#include "Game.hpp"
#include "Game/Player.hpp"
#include "Game/Team.hpp"

void game::GameLogic::GameLoop() {
  // readCommands();
  // execCommands();
  lifeUpdate();
  ressourcesUpdate();
  // checkWinCond();
}
