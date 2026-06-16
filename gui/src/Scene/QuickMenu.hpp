#pragma once
#include "IScene/IScene.hpp"
#include "Texture/TextureManager.hpp"
#include "Sprite/Sprite.hpp"
#include "Logger.hpp"
#include "Render/Render.hpp"
#include <memory>
#include <vector>
#include <map>
#include "Utils/math.hpp"
#include <algorithm>

namespace Zappy {
    class quickMenu : public IScene {
        private:
            TextureManager &_texManager;
            std::unique_ptr<Sprite> _backgroundSprite;
            std::vector<std::unique_ptr<Sprite>> _buttons;
            std::unique_ptr<Shader> _uiShader;
        public:
            quickMenu(TextureManager &tm) : _texManager(tm) {}
            void onEnter() override {
                Texture& menuTex = _texManager.get("gui/assets/quickMenu.png");
                _uiShader = std::make_unique<Shader>("gui/src/Core/Shader/ui.vert", "gui/src/Core/Shader/ui.frag");
                _backgroundSprite = std::make_unique<Sprite>(menuTex);
                _backgroundSprite->setPosition(960.0f, 332.5f);
                _backgroundSprite->scale = Zappy::Math::vec3(415.0f, 415.0f, 1.0f);
            }
            SceneState update(const std::vector<Zappy::Event> &events, const Zappy::GameState &gameState, const std::vector<Zappy::NetworkEvent> &netEvents, float deltaTime) override 
            {
                return SceneState::NONE;
            }
            void draw(Shader &shader) override {
                if (!_backgroundSprite || !_uiShader)
                    return;
                glDisable(GL_DEPTH_TEST);
                _uiShader->bind();
                Zappy::Math::mat4 orthoProjection = Zappy::Math::ortho(0.0f, WIDTH, 0.0f, HEIGHT, -1.0f, 1.0f);
                Zappy::Math::mat4 view;
                _backgroundSprite->draw(*_uiShader, view, orthoProjection);
                glEnable(GL_DEPTH_TEST);
            }
            void onExit() override {
                _backgroundSprite.reset();
                _uiShader.reset();
            }
    };
}
