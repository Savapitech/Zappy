#pragma once

namespace Zappy
{
    enum class EventType {
        WindowClosed,
        KeyPressed,
        KeyReleased,
        MousePressed,
        MouseReleased,
        MouseMoved
    };

    struct Event {
        EventType type;

        int mouseX = 0;
        int mouseY = 0;
        int button = 0;
        int keyCode = 0;
        };
}
