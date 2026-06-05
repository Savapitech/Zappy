#pragma once
#include <string>
#include <vector>

namespace Zappy {

    enum class NetworkEventType {
        BROADCAST,
        INCANTATION_START,
        INCANTATION_END,
        PLAYER_DEATH,
        EGG_LAID,
        SERVER_MESSAGE,
        UNKNOWN_COMMAND
    };

    struct NetworkEvent {
        NetworkEventType type;
        std::vector<std::string> arguments;
    };

} // namespace Zappy