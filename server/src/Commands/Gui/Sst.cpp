#include "Sst.hpp"
#include "Client.hpp"
#include "Game/GameLogic.hpp"
#include "Server.hpp"

namespace commands {
namespace gui {
void Sst::execute(std::shared_ptr<Client> client,
                  std::vector<std::string> &args) {
  if (args.empty()) {
    client->sendMessage("sbp\n");
    return;
  }

  game::GameLogic &game = client->getServer().get().getGame();
  int freq;

  try {
    freq = std::stoi(args[0]);
  } catch (const std::exception &) {
    client->sendMessage("sbp\n");
    return;
  }

  if (freq < 1) {
    client->sendMessage("sbp\n");
    return;
  }

  game.setFreq(freq);
  client->sendMessage("sst " + std::to_string(freq) + "\n");
}
} // namespace gui
} // namespace commands
