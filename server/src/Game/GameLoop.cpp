#include "GameLogic.hpp"

void game::GameLogic::GameLoop() {
  // readCommands();
  // execCommands();
  lifeUpdate();
  ressourcesUpdate();
  checkWinCond();
}
