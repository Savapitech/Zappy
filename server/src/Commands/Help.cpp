#include <iostream>

#include "Help.hpp"

namespace commands {
  void Help::execute(std::shared_ptr<Client> client,
                   std::vector<std::string> &args) {
    std::cout << "Pouet help" << std::endl;
  }
}
