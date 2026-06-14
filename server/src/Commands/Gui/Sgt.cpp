#include "Sgt.hpp"
#include "Client.hpp"
#include "Game/GameLogic.hpp"
#include "Server.hpp"

namespace commands {
namespace gui {
void Sgt::execute(std::shared_ptr<Client> client,
                  std::vector<std::string> &args) {
  (void)args;
  game::GameLogic &game = client->getServer().get().getGame();

  client->sendMessage("sgt " + std::to_string(game.getFreq()) + "\n");
}
} // namespace gui
} // namespace commands
