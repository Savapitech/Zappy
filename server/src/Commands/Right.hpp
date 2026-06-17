#pragma once

#include "ACommand.hpp"
#include "Game/Common.hpp"

namespace commands {

class Right : public ACommand {
  void execute(std::shared_ptr<Client> client,
               std::vector<std::string> &args) override;
  int getCost() const override { return TIME_RIGHT; }
};

} // namespace commands
