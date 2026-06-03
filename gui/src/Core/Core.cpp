#define GL_GLEXT_PROTOTYPES 1

#include "Core.hpp"
#include "Logger.hpp"

#include "SceneManager/SceneManager.hpp"

#include <GL/gl.h>
#include <GL/glx.h>

namespace Zappy {

    Core::Core() : _isRunning(true) {}

    Core::~Core() {}

    void Core::init() {
        _window.open(WIDTH, HEIGHT, "Zappy");

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

        _defaultShader = std::make_unique<Zappy::Shader>("gui/src/Core/Shader/vertex.vert", "gui/src/Core/Shader/fragment.frag");

        _sceneManager.changeScene(std::make_unique<MenuScene>(_sceneManager.getTextureManager()));
    }

    void Core::run() {
        while (_isRunning) {
            const auto& events = _window.pollEvents();
            for (const auto& event : events) {
                if (event.type == Zappy::EventType::WindowClosed) {
                    _isRunning = false;
                }
            }

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            _sceneManager.update();
            _defaultShader->bind();
            _sceneManager.draw(*_defaultShader);
            _window.swapBuffers();
        }
    }

}