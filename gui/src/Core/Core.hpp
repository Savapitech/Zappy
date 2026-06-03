#pragma once

#include "../Window/Window.hpp"
#include "Texture/Texture.hpp"
#include "Texture/TextureManager.hpp"
#include "SceneManager/SceneManager.hpp"

#include "Core/Shader/Shader.hpp"
#include "Sprite/Sprite.hpp"

#include <memory>

namespace Zappy {
    class Core
    {
    private:
        Zappy::SceneManager _sceneManager;
        std::unique_ptr<Zappy::Shader> _defaultShader;
        bool _isRunning;
        Zappy::Window _window;
    public:
        Core();
        ~Core();

        void init();
        void run(void);
    };
}
