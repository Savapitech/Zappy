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
  
  Zappy::Math::vec2 _uvOffset = Zappy::Math::vec2(0.0f, 0.0f);
  Zappy::Math::vec2 _uvScale = Zappy::Math::vec2(1.0f, 1.0f);

  Sprite(Texture &texture);
  Sprite(Texture &texture, Zappy::Math::vec2 uvScale, Zappy::Math::vec2 uvOffset);
  ~Sprite();

  void setTexture(Texture &newTexture);
  void setPosition(float x, float y);
  void draw(Shader &shader, const Zappy::Math::mat4 &view,
            const Zappy::Math::mat4 &projection);
};
} // namespace Zappy