#include "Inventory.hpp"
#include "Client.hpp"
#include "Game/GameLogic.hpp"
#include "Server.hpp"

namespace commands {
void Inventory::execute(std::shared_ptr<Client> client,
                        std::vector<std::string> &args) {
  (void)args;
  client->getServer().get().getGame().playerInventory(*client->getPlayer());
}
} // namespace commands
