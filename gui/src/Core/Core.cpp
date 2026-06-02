#define GL_GLEXT_PROTOTYPES 1

#include "Core.hpp"
#include "Logger.hpp"

#include <GL/gl.h>
#include <GL/glx.h>
#include <Utils/math.hpp>

namespace Zappy {

    Core::Core() : _isRunning(true) {}

    Core::~Core() {}

    void Core::init() {
        _window.open(WIDTH, HEIGHT, "Zappy");

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

        _myTexture = std::make_unique<Zappy::Texture>("/media/data/zappy/Zappy/gui/assets/cute.png");
        _defaultShader = std::make_unique<Zappy::Shader>("gui/src/Core/Shader/vertex.vert", "gui/src/Core/Shader/fragment.frag");

        auto myFirstSprite = std::make_unique<Zappy::Sprite>(*_myTexture);

        myFirstSprite->position = Zappy::Math::vec3(0.0f, 0.0f, -100.0f);
        myFirstSprite->scale = Zappy::Math::vec3(5.0f, 5.0f, 1.0f);

        _sprites.push_back(std::move(myFirstSprite));
    }

    void Core::run() {
        while (_isRunning) {
            const auto& events = _window.pollEvents();
            for (const auto& event : events)
                if (event.type == Zappy::EventType::WindowClosed)
                    _isRunning = false;

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            Zappy::Math::mat4 projection = Zappy::Math::perspective(
                Zappy::Math::radians(45.0f),
                static_cast<float>(WIDTH) / static_cast<float>(HEIGHT),
                0.1f, 1000.0f
            );

            Zappy::Math::mat4 view = Zappy::Math::translate(
                Zappy::Math::mat4(),
                Zappy::Math::vec3(0.0f, 0.0f, -3.0f)
            );

            Zappy::Math::mat4 viewProj = projection * view;

            _defaultShader->bind();

            for (auto& sprite : _sprites) {
                sprite->draw(*_defaultShader, viewProj);
            }

            _window.swapBuffers();
        }
    }

}