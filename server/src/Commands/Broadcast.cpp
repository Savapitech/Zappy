#include "Broadcast.hpp"
#include "Client.hpp"
#include "Game/GameLogic.hpp"
#include "Server.hpp"

namespace commands {
void Broadcast::execute(std::shared_ptr<Client> client,
                        std::vector<std::string> &args) {
  std::string text;

  for (size_t i = 0; i < args.size(); i++) {
    if (i > 0)
      text += " ";
    text += args[i];
  }

  client->getServer().get().getGame().playerBroadcast(*client->getPlayer(),
                                                      text);
}
} // namespace commands
