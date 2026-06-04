#include "./Core/Core.hpp"
#include "Logger.hpp"

int main(void) {
  Logger::setLevel(L_DEBUG);
  LOG_DEBUG("Start gui");

  try {
    Zappy::Core core;

    core.init();
    core.run();

  } catch (const std::exception &e) {
    LOG_FATAL(e.what());
  }
  LOG_DEBUG("End gui");
}