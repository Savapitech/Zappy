#include "Core/Core.hpp"
#include "Logger.hpp"

int main(int ac, char **av) {
  Logger::setLevel(L_DEBUG);
  LOG_DEBUG("Start gui");

  try {
    Zappy::Core core;

    std::string ip;
    std::string machine;

    if (ac == 5){
      if (std::string(av[1]) == "-p")
        ip = av[2];
      if (std::string(av[3]) == "-h")
        machine = av[4];
    } else {
        throw std::runtime_error("USAGE: ./zappy_gui -p port -h machine");
    }

    try
    {
      core.init(ip, std::stoi(machine));
    }
    catch(const std::exception& e)
    {
      throw std::runtime_error("USAGE: ./zappy_gui -p port -h machine");
    }
    
    core.run();

  } catch (const std::exception &e) {
    LOG_FATAL(e.what());
  }
  LOG_DEBUG("End gui");
}