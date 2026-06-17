#include "Take.hpp"
#include "Client.hpp"
#include "Game/GameLogic.hpp"
#include "Server.hpp"

namespace commands {
void Take::execute(std::shared_ptr<Client> client,
                   std::vector<std::string> &args) {
  if (args.empty()) {
    client->sendMessage("ko\n");
    return;
  }

  client->getServer().get().getGame().playerTakeRessources(*client->getPlayer(),
                                                           args[0]);
}
} // namespace commands
