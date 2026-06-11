#include "Tna.hpp"
#include "Client.hpp"
#include "Game/GameLogic.hpp"
#include "Server.hpp"

namespace commands {
namespace gui {
void Tna::execute(std::shared_ptr<Client> client,
                  std::vector<std::string> &args) {
  (void)args;
  game::GameLogic &game = client->getServer().get().getGame();

  for (const auto &team : game.getTeams())
    client->sendMessage("tna " + team->getName() + "\n");
}
} // namespace gui
} // namespace commands
