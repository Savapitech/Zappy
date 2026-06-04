
#pragma once

#include "Core/Shader/Shader.hpp"
#include "Event.hpp"

#define WIDTH 1920.0f
#define HEIGHT 1080.0f

namespace Zappy {
enum class SceneState { NONE, MENU, GAME };
}

namespace Zappy {
class IScene {
public:
  virtual ~IScene() = default;

  virtual void onEnter() = 0;
  virtual SceneState update(const std::vector<Zappy::Event> &events, float deltaTime) = 0;
  virtual void draw(Shader &shader) = 0;
  virtual void onExit() = 0;
};
} // namespace Zappy
