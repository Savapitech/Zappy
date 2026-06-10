#pragma once
#include <string>
#include <vector>

namespace Zappy {

    enum class NetworkEventType {
        PLAYER_CONNECTED,
        PLAYER_DISCONNECTED,
        BROADCAST,
        INCANTATION_START,
        INCANTATION_END,
        EGG_LAID,
        EGG_HATCHED,
        EGG_DIED,
        GAME_OVER,
        SERVER_MESSAGE,
        TILE_UPDATED,
        UNKNOWN_COMMAND
    };

    struct NetworkEvent {
        NetworkEventType type;
        std::vector<std::string> arguments;
    };

} // namespace Zappy