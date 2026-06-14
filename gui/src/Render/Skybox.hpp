#pragma once
#include "../Utils/OpenGL.hpp"
#include <string>
#include <vector>

namespace Zappy {
class Skybox {
private:
  unsigned int skyboxVAO, skyboxVBO;
  unsigned int cubemapTexture;

  unsigned int loadCubemap(const std::vector<std::string> &faces);
  void setupMesh();

public:
  Skybox(const std::vector<std::string> &faces);
  ~Skybox();

  void draw() const;
};
} // namespace Zappy