#include "Set.hpp"
#include "Client.hpp"
#include "Game/GameLogic.hpp"
#include "Server.hpp"

namespace commands {
void Set::execute(std::shared_ptr<Client> client,
                  std::vector<std::string> &args) {
  if (args.empty()) {
    client->sendMessage("ko\n");
    return;
  }

  client->getServer().get().getGame().playerDropRessources(*client->getPlayer(),
                                                           args[0]);
}
} // namespace commands
