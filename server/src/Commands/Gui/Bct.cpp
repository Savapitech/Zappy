#include "Bct.hpp"
#include "Client.hpp"
#include "Game/GameLogic.hpp"
#include "Server.hpp"

namespace commands {
namespace gui {
void Bct::execute(std::shared_ptr<Client> client,
                  std::vector<std::string> &args) {
  if (args.size() < 2) {
    client->sendMessage("sbp\n");
    return;
  }

  game::GameLogic &game = client->getServer().get().getGame();
  int x;
  int y;

  try {
    x = std::stoi(args[0]);
    y = std::stoi(args[1]);
  } catch (const std::exception &) {
    client->sendMessage("sbp\n");
    return;
  }

  client->sendMessage(game.formatBct(x, y));
}
} // namespace gui
} // namespace commands
