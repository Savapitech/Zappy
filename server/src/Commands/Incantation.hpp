#pragma once

#include "ACommand.hpp"
#include "Game/Common.hpp"

namespace commands {

class Incantation : public ACommand {
  bool _started = false;

  void execute(std::shared_ptr<Client> client,
               std::vector<std::string> &args) override;
  int getCost() const override { return TIME_INCANTATION; }
  void onQueued(std::shared_ptr<Client> client) override;
};

} // namespace commands
