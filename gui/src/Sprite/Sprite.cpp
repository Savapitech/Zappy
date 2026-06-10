#define GL_GLEXT_PROTOTYPES 1

#include "Sprite.hpp"

#include "Utils/OpenGL.hpp"

namespace Zappy {
Sprite::Sprite(Texture &texture)
    : _VAO(0), _VBO(0), _EBO(0), _texture(texture), position(0.0f, 0.0f, 0.0f),
      scale(1.0f, 1.0f, 1.0f) {
  float vertices[] = {0.5f,  1.0f, 1.0f, 1.0f, 0.5f,  0.0f, 1.0f, 0.0f,
                      -0.5f, 0.0f, 0.0f, 0.0f, -0.5f, 1.0f, 0.0f, 1.0f};
  unsigned int indices[] = {0, 1, 3, 1, 2, 3};

  glGenVertexArrays(1, &_VAO);
  glGenBuffers(1, &_VBO);
  glGenBuffers(1, &_EBO);

  glBindVertexArray(_VAO);

  glBindBuffer(GL_ARRAY_BUFFER, _VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
               GL_STATIC_DRAW);

  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                        (void *)(2 * sizeof(float)));
  glEnableVertexAttribArray(1);

  glBindVertexArray(0);
}

Sprite::~Sprite() {
  if (_VAO != 0)
    glDeleteVertexArrays(1, &_VAO);
  if (_VBO != 0)
    glDeleteBuffers(1, &_VBO);
  if (_EBO != 0)
    glDeleteBuffers(1, &_EBO);
}

void Sprite::setTexture(Texture &newTexture) { _texture = newTexture; }

void Sprite::draw(Shader &shader, const Zappy::Math::mat4 &view,
                  const Zappy::Math::mat4 &projection) {
  Zappy::Math::mat4 model;
  model = Zappy::Math::translate(model, position);

  float texWidth = static_cast<float>(_texture.get().getWidth());
  float texHeight = static_cast<float>(_texture.get().getHeight());
  float ratio = (texHeight > 0.0f) ? (texWidth / texHeight) : 1.0f;

  shader.setFloat("u_uvScaleX", 1.0f);
  shader.setFloat("u_uvScaleY", 1.0f);
  shader.setFloat("u_uvOffsetX", 0.0f);
  shader.setFloat("u_uvOffsetY", 0.0f);

  if (isBillboard) {
    Zappy::Math::mat4 modelView = view * model;

    modelView.m[0] = scale.x * ratio;
    modelView.m[1] = 0.0f;
    modelView.m[2] = 0.0f;
    modelView.m[4] = 0.0f;
    modelView.m[5] = scale.y;
    modelView.m[6] = 0.0f;
    modelView.m[8] = 0.0f;
    modelView.m[9] = 0.0f;
    modelView.m[10] = 1.0f;

    Zappy::Math::mat4 mvp = projection * modelView;
    shader.setMat4("u_MVP", mvp);
    shader.setMat4("u_Model", model);
  } else {
    if (rotation.x != 0.0f)
      model = Zappy::Math::rotateX(model, rotation.x);
    if (rotation.y != 0.0f)
      model = Zappy::Math::rotateY(model, rotation.y);
    if (rotation.z != 0.0f)
      model = Zappy::Math::rotateZ(model, rotation.z);

    model = Zappy::Math::scale(
        model, Zappy::Math::vec3(scale.x * ratio, scale.y, 1.0f));

    Zappy::Math::mat4 mvp = projection * view * model;
    shader.setMat4("u_MVP", mvp);
    shader.setMat4("u_Model", model);
  }
  glActiveTexture(GL_TEXTURE0);
  shader.setInt("ourTexture", 0);

  _texture.get().bind();
  glBindVertexArray(_VAO);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void Sprite::setPosition(float x, float y)
{
  position.x = x;
  position.y = y;
}

} // namespace Zappy