#pragma once
#include "Core/Shader/Shader.hpp"
#include "Event.hpp"
#include <vector>

namespace Zappy {
class IButton {
private:
public:
  virtual ~IButton() = default;
  virtual void draw(Shader &shader) = 0;
  virtual void setPosition(float x, float y) = 0;
  virtual void update(const std::vector<Zappy::Event> &events) = 0;
};
} // namespace Zappy
