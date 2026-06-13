#pragma once
#include <string>
#include <vector>

namespace Zappy {

    enum class NetworkEventType {
        PLAYER_CONNECTED,
        PLAYER_DISCONNECTED,
        PLAYER_MOVED,
        PLAYER_LEVEL_UP,
        BROADCAST,
        INCANTATION_START,
        INCANTATION_END,
        EGG_LAID,
        EGG_HATCHED,
        EGG_DIED,
        GAME_OVER,
        SERVER_MESSAGE,
        TILE_UPDATED,
        RESOURCE_COLLECTED,
        RESOURCE_DROPPED,
        PLAYER_EXPULSED,
        EGG_LAYING,
        TIME_UNIT_MODIFIED,
        SERVER_ERROR,
        UNKNOWN_COMMAND

    };

    struct NetworkEvent {
        NetworkEventType type;
        std::vector<std::string> arguments;
    };

} // namespace Zappy