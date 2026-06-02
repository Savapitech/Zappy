#pragma once

#include "Event.hpp"

#include <string>
#include <vector>

namespace Zappy {
class Window {
private:
    void* _display;
    unsigned long _windowHandle;
    void* _context;
    unsigned long _wmDeleteMessage;

    std::vector<Zappy::Event> _events;

public:
    Window();
    ~Window();

    void open(unsigned int width, unsigned int height, const std::string& title);
    void close();
    void swapBuffers();

    const std::vector<Zappy::Event>& pollEvents();
};
}