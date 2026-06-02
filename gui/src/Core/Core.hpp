#pragma once

#include "../Window/Window.hpp"
#include "Texture/Texture.hpp"

#include <memory>

class Core
{
    private:
        //Network
        //ServerParser
        Zappy::Window _window;
        //GuiManager
        //GameState
        bool _isRunning;

        std::unique_ptr<Zappy::Texture> _myTexture;

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
