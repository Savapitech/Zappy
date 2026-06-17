#include "Mct.hpp"
#include "Client.hpp"
#include "Game/GameLogic.hpp"
#include "Server.hpp"

namespace commands {
namespace gui {
void Mct::execute(std::shared_ptr<Client> client,
                  std::vector<std::string> &args) {
  (void)args;
  game::GameLogic &game = client->getServer().get().getGame();

  for (int y = 0; y < game.getMapY(); y++)
    for (int x = 0; x < game.getMapX(); x++)
      client->sendMessage(game.formatBct(x, y));
}
} // namespace gui
} // namespace commands
