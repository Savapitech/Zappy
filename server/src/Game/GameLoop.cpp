#include "GameLogic.hpp"

void game::GameLogic::Debug() { return; }

void game::GameLogic::Poll() {
  lifeUpdate();
  ressourcesUpdate();
  checkWinCond();
  Debug();
}
