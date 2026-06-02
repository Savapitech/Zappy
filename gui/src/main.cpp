#include "Logger.hpp"
#include "./Core/Core.hpp"

int main(void)
{
    Logger::setLevel(L_DEBUG);
    LOG_DEBUG("Start gui");

    try
    {
        Core core;

        while (1)
        {
        }

    }catch(const std::exception& e)
    {
        LOG_FATAL(e.what());
    }
    LOG_DEBUG("End gui");
}