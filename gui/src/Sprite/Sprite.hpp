#pragma once

#include "Texture/Texture.hpp"
#include "Core/Shader/Shader.hpp"
#include "Utils/math.hpp"

#include <memory>
#include <functional>

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

        Sprite(Texture& texture);
        ~Sprite();

        void setTexture(Texture& newTexture);
        void draw(Shader& shader, const Zappy::Math::mat4& view, const Zappy::Math::mat4& projection);
    };
}