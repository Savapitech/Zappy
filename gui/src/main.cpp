#include "Logger.hpp"
#include "./Window/Window.hpp"

int main(void)
{
    Logger::setLevel(L_DEBUG);
    LOG_DEBUG("Start gui");

    Zappy::Window window;

    window.open(1920, 1080, "Zappy");
    while (1)
    {
    }
    
    LOG_DEBUG("End gui");
}