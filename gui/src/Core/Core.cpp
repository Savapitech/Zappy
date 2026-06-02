#include "Core.hpp"



Core::Core()
{
    _window.open(1920, 1080, "Zappy");
}

Core::~Core()
{
    _window.close();
}