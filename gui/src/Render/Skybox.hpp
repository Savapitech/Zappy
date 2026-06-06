#pragma once
#include <vector>
#include <string>
#include "../Utils/OpenGL.hpp"

namespace Zappy {
class Skybox {
private:
    unsigned int skyboxVAO, skyboxVBO;
    unsigned int cubemapTexture;

    unsigned int loadCubemap(const std::vector<std::string>& faces);
    void setupMesh();

public:
    Skybox(const std::vector<std::string>& faces);
    ~Skybox();

    void draw() const;
};
} // namespace Zappy