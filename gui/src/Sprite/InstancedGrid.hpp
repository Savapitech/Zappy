#pragma once
#include "Core/Shader/Shader.hpp"
#include "Texture/Texture.hpp"
#include "Utils/OpenGL.hpp"
#include "Utils/math.hpp"
#include <vector>

namespace Zappy {
class InstancedGrid {
private:
  unsigned int _VAO, _VBO, _EBO, _instanceVBO;
  std::reference_wrapper<Texture> _texture;
  std::vector<Zappy::Math::mat4> _matrices;

public:
  InstancedGrid(Texture &texture) : _texture(texture) {
    float vertices[] = {0.5f,  1.0f, 1.0f, 1.0f, 0.5f,  0.0f, 1.0f, 0.0f,
                        -0.5f, 0.0f, 0.0f, 0.0f, -0.5f, 1.0f, 0.0f, 1.0f};
    unsigned int indices[] = {0, 1, 3, 1, 2, 3};

    glGenVertexArrays(1, &_VAO);
    glGenBuffers(1, &_VBO);
    glGenBuffers(1, &_EBO);
    glGenBuffers(1, &_instanceVBO);

    glBindVertexArray(_VAO);

    glBindBuffer(GL_ARRAY_BUFFER, _VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
                 GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                          (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                          (void *)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, _instanceVBO);

    std::size_t vec4Size = 4 * sizeof(float);
    for (int i = 0; i < 4; i++) {
      glEnableVertexAttribArray(2 + i);
      glVertexAttribPointer(2 + i, 4, GL_FLOAT, GL_FALSE, 16 * sizeof(float),
                            (void *)(i * vec4Size));
      glVertexAttribDivisor(2 + i, 1);
    }

    glBindVertexArray(0);
  }

  ~InstancedGrid() {
    glDeleteVertexArrays(1, &_VAO);
    glDeleteBuffers(1, &_VBO);
    glDeleteBuffers(1, &_EBO);
    glDeleteBuffers(1, &_instanceVBO);
  }

  void addTile(const Zappy::Math::vec3 &position, float scale) {
    Zappy::Math::mat4 model;
    model = Zappy::Math::translate(model, position);
    model = Zappy::Math::rotateX(model, Zappy::Math::radians(-90.0f));
    model = Zappy::Math::scale(model, Zappy::Math::vec3(scale, scale, 1.0f));
    _matrices.push_back(model);
  }

  void build() {
    glBindVertexArray(_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, _instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, _matrices.size() * 16 * sizeof(float),
                 _matrices[0].value_ptr(), GL_STATIC_DRAW);
    glBindVertexArray(0);
  }

  void draw(Shader &shader, const Zappy::Math::mat4 &viewProj) {
    shader.setMat4("u_ViewProj", viewProj);
    _texture.get().bind();

    glBindVertexArray(_VAO);
    glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0,
                            _matrices.size());
    glBindVertexArray(0);
  }
};
} // namespace Zappy