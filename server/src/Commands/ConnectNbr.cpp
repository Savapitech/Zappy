#include "ConnectNbr.hpp"
#include "Client.hpp"
#include "Game/GameLogic.hpp"
#include "Server.hpp"

namespace commands {
void ConnectNbr::execute(std::shared_ptr<Client> client,
                         std::vector<std::string> &args) {
  (void)args;
  client->getServer().get().getGame().playerConnectNbr(*client->getPlayer());
}
} // namespace commands
