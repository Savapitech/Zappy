#include "Fork.hpp"
#include "Client.hpp"
#include "Game/GameLogic.hpp"
#include "Server.hpp"

namespace commands {
void Fork::execute(std::shared_ptr<Client> client,
                   std::vector<std::string> &args) {
  (void)args;
  client->getServer().get().getGame().playerFork(*client->getPlayer());
}
} // namespace commands
