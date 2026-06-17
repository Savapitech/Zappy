#include "Incantation.hpp"
#include "Client.hpp"
#include "Game/GameLogic.hpp"
#include "Server.hpp"

namespace commands {
void Incantation::execute(std::shared_ptr<Client> client,
                          std::vector<std::string> &args) {
  (void)args;
  if (_started)
    client->getServer().get().getGame().playerIncantationEnd(
        *client->getPlayer());
}

void Incantation::onQueued(std::shared_ptr<Client> client) {
  _started = client->getServer().get().getGame().playerIncantationStart(
      *client->getPlayer());
}
} // namespace commands
