#include "Right.hpp"
#include "Client.hpp"
#include "Game/GameLogic.hpp"
#include "Server.hpp"

namespace commands {
void Right::execute(std::shared_ptr<Client> client,
                    std::vector<std::string> &args) {
  (void)args;
  client->getServer().get().getGame().playerTurnRight(*client->getPlayer());
}
} // namespace commands
