#pragma once

#include "../Window/Window.hpp"
#include "Texture/Texture.hpp"
#include "Texture/TextureManager.hpp"

#include "Core/Shader/Shader.hpp"
#include "Sprite/Sprite.hpp"

#include <memory>

namespace Zappy {
    class Core
    {
        private:
            //Network
            //ServerParser
            Zappy::Window _window;
            //GuiManager
            //GameState
            bool _isRunning;

            std::unique_ptr<Zappy::Shader> _defaultShader;
            TextureManager _textureManager;
            std::vector<std::unique_ptr<Zappy::Sprite>> _sprites;

            unsigned int _shaderProgram;
            unsigned int _VAO;
            unsigned int _VBO;
            unsigned int _EBO;
        public:
            Core();
            ~Core();

            void init();
            void run(void);
    };
}