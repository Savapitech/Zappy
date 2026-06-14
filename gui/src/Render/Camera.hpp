#pragma once
#include "Event.hpp"
#include "Utils/math.hpp"
#include <cmath>
#include <vector>

namespace Zappy {
class Camera {
public:
  Zappy::Math::vec3 position = {0.0f, 25.0f, 35.0f};

  float yaw = -90.0f;
  float pitch = -35.0f;

  float speed = 0.5f;
  float sensitivity = 2.0f;

  void update(const std::vector<Zappy::Event> &events) {
    for (const auto &event : events) {
      if (event.type == Zappy::EventType::KeyPressed) {
        Zappy::Math::vec3 front = getFront();
        Zappy::Math::vec3 right = Zappy::Math::normalize(
            Zappy::Math::cross(front, Zappy::Math::vec3(0, 1, 0)));

        if (event.keyCode == Zappy::Key::Z) {
          position.x += front.x * speed;
          position.z += front.z * speed;
        }
        if (event.keyCode == Zappy::Key::S) {
          position.x -= front.x * speed;
          position.z -= front.z * speed;
        }
        if (event.keyCode == Zappy::Key::Q) {
          position.x -= right.x * speed;
          position.z -= right.z * speed;
        }
        if (event.keyCode == Zappy::Key::D) {
          position.x += right.x * speed;
          position.z += right.z * speed;
        }

        if (event.keyCode == Zappy::Key::Left)
          yaw -= sensitivity;
        if (event.keyCode == Zappy::Key::Right)
          yaw += sensitivity;
        if (event.keyCode == Zappy::Key::Up)
          pitch += sensitivity;
        if (event.keyCode == Zappy::Key::Down)
          pitch -= sensitivity;

        if (pitch > 89.0f)
          pitch = 89.0f;
        if (pitch < -89.0f)
          pitch = -89.0f;
      }
    }
  }

  Zappy::Math::vec3 getFront() const {
    Zappy::Math::vec3 front;
    front.x = std::cos(Zappy::Math::radians(yaw)) *
              std::cos(Zappy::Math::radians(pitch));
    front.y = std::sin(Zappy::Math::radians(pitch));
    front.z = std::sin(Zappy::Math::radians(yaw)) *
              std::cos(Zappy::Math::radians(pitch));
    return Zappy::Math::normalize(front);
  }

  Zappy::Math::mat4 getViewMatrix() const {
    Zappy::Math::vec3 front = getFront();
    Zappy::Math::vec3 target = {position.x + front.x, position.y + front.y,
                                position.z + front.z};
    return Zappy::Math::lookAt(position, target, Zappy::Math::vec3(0, 1, 0));
  }
};
} // namespace Zappy