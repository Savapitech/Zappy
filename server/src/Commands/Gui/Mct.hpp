#pragma once

#include "Commands/ACommand.hpp"

namespace commands {
namespace gui {

class Mct : public ACommand {
  void execute(std::shared_ptr<Client> client,
               std::vector<std::string> &args) override;
};

} // namespace gui
} // namespace commands
