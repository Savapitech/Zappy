#include "Look.hpp"
#include "Client.hpp"
#include "Game/GameLogic.hpp"
#include "Server.hpp"

namespace commands {
void Look::execute(std::shared_ptr<Client> client,
                   std::vector<std::string> &args) {
  (void)args;
  client->getServer().get().getGame().playerLook(*client->getPlayer());
}
} // namespace commands
