#include "Incantation.hpp"
#include "Client.hpp"
#include "Game/GameLogic.hpp"
#include "Server.hpp"

namespace commands {
void Incantation::execute(std::shared_ptr<Client> client,
                          std::vector<std::string> &args) {
  (void)args;
  client->getServer().get().getGame().playerIncantation(*client->getPlayer());
}

void Incantation::onQueued(std::shared_ptr<Client> client) {
  client->sendMessage("Elevation underway\n");
}
} // namespace commands
