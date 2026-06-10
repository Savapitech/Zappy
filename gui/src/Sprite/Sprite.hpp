#pragma once

#include "Core/Shader/Shader.hpp"
#include "Texture/Texture.hpp"
#include "Utils/math.hpp"

#include <functional>
#include <memory>

namespace Zappy {
class Sprite {
private:
  unsigned int _VAO;
  unsigned int _VBO;
  unsigned int _EBO;

  std::reference_wrapper<Texture> _texture;

public:
  Zappy::Math::vec3 rotation;
  Zappy::Math::vec3 position;
  Zappy::Math::vec3 scale;
  bool isBillboard;

  Sprite(Texture &texture);
  ~Sprite();

  void setTexture(Texture &newTexture);
  void setPosition(float x, float y);
  void draw(Shader &shader, const Zappy::Math::mat4 &view,
            const Zappy::Math::mat4 &projection);
};
} // namespace Zappy