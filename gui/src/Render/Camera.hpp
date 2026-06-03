#pragma once
#include "Event.hpp"
#include "Utils/math.hpp"
#include <vector>

namespace Zappy {
class Camera {
public:
  Zappy::Math::vec3 position = {0.0f, 25.0f, 35.0f};
  Zappy::Math::vec3 target = {0.0f, 0.0f, 0.0f};
  float speed = 0.5f;

  void update(const std::vector<Zappy::Event> &events) {
    for (const auto &event : events) {
      if (event.type == Zappy::EventType::KeyPressed) {
        if (event.keyCode == Zappy::Key::Z)
          position.z -= speed;
        if (event.keyCode == Zappy::Key::S)
          position.z += speed;
        if (event.keyCode == Zappy::Key::Q)
          position.x -= speed;
        if (event.keyCode == Zappy::Key::D)
          position.x += speed;
      }
    }
  }

  Zappy::Math::mat4 getViewMatrix() const {
    return Zappy::Math::lookAt(position, target, Zappy::Math::vec3(0, 1, 0));
  }
};
} // namespace Zappy