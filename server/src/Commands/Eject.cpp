#include "Eject.hpp"
#include "Client.hpp"
#include "Game/GameLogic.hpp"
#include "Server.hpp"

namespace commands {
void Eject::execute(std::shared_ptr<Client> client,
                    std::vector<std::string> &args) {
  (void)args;
  client->getServer().get().getGame().playerEject(*client->getPlayer());
}
} // namespace commands
