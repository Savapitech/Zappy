#include "Pin.hpp"
#include "Client.hpp"
#include "Game/GameLogic.hpp"
#include "Game/Player.hpp"
#include "Server.hpp"

namespace commands {
namespace gui {
void Pin::execute(std::shared_ptr<Client> client,
                  std::vector<std::string> &args) {
  if (args.empty() || args[0].empty() || args[0][0] != '#') {
    client->sendMessage("sbp\n");
    return;
  }

  game::GameLogic &game = client->getServer().get().getGame();
  int id;

  try {
    id = std::stoi(args[0].substr(1));
  } catch (const std::exception &) {
    client->sendMessage("sbp\n");
    return;
  }

  auto player = game.getPlayerById(id);
  if (!player) {
    client->sendMessage("sbp\n");
    return;
  }

  const auto &inv = player->getInventory();
  std::string msg = "pin #" + std::to_string(player->getId()) + " " +
                    std::to_string(player->getX()) + " " +
                    std::to_string(player->getY());

  for (int i = 0; i < RESOURCE_COUNT; i++)
    msg += " " + std::to_string(inv[i]);
  msg += "\n";

  client->sendMessage(msg);
}
} // namespace gui
} // namespace commands
