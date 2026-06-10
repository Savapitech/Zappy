
#pragma once

#include "Core/Shader/Shader.hpp"
#include "Event.hpp"
#include "GameState.hpp"
#include "Network/NetworkEvent.hpp"

#define WIDTH 1920.0f
#define HEIGHT 1080.0f

namespace Zappy {
enum class SceneState { NONE, INTRO, TITLE, MENU, GAME };


class IScene {
public:
  virtual ~IScene() = default;

  virtual void onEnter() = 0;
  virtual SceneState update(const std::vector<Zappy::Event> &events, 
                            const Zappy::GameState &gameState,
                            const std::vector<Zappy::NetworkEvent> &netEvents,
                           float deltaTime) = 0;
  virtual void draw(Shader &shader) = 0;
  virtual void onExit() = 0;
};
} // namespace Zappy