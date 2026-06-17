#pragma once

#include <memory>
#include <string>
#include <vector>

class Client;

namespace commands {

class ICommand {
public:
  virtual ~ICommand() = default;
  virtual void execute(std::shared_ptr<Client> client,
                       std::vector<std::string> &args) = 0;

  // time, in time units, the command takes to execute (action / f seconds)
  virtual int getCost() const { return 0; }

  // called once the command starts executing (timer just started)
  virtual void onQueued(std::shared_ptr<Client> client) { (void)client; }
};

} // namespace commands
