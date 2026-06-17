#include "Msz.hpp"
#include "Client.hpp"
#include "Game/GameLogic.hpp"
#include "Server.hpp"

namespace commands {
namespace gui {
void Msz::execute(std::shared_ptr<Client> client,
                  std::vector<std::string> &args) {
  (void)args;
  game::GameLogic &game = client->getServer().get().getGame();

  client->sendMessage("msz " + std::to_string(game.getMapX()) + " " +
                      std::to_string(game.getMapY()) + "\n");
}
} // namespace gui
} // namespace commands
