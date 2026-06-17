#pragma once

#include "ACommand.hpp"

namespace commands {

class Incantation : public ACommand {
  void execute(std::shared_ptr<Client> client,
               std::vector<std::string> &args) override;
};

} // namespace commands
