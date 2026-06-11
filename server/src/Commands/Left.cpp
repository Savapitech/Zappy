#include "Left.hpp"
#include "Client.hpp"
#include "Game/GameLogic.hpp"
#include "Server.hpp"

namespace commands {
void Left::execute(std::shared_ptr<Client> client,
                   std::vector<std::string> &args) {
  (void)args;
  client->getServer().get().getGame().playerTurnLeft(*client->getPlayer());
}
} // namespace commands
