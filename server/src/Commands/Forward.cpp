#include "Forward.hpp"
#include "Client.hpp"
#include "Game/GameLogic.hpp"
#include "Server.hpp"

namespace commands {
void Forward::execute(std::shared_ptr<Client> client,
                      std::vector<std::string> &args) {
  (void)args;
  client->getServer().get().getGame().playerForward(*client->getPlayer());
}
} // namespace commands
