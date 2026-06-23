#pragma once

#include "IScene/IScene.hpp"
#include "Texture/TextureManager.hpp"
#include "Sprite/Sprite.hpp"
#include "Logger.hpp"
#include "Buttons/Button.hpp"
#include "Network/NetworkManager.hpp"
#include "Render/Render.hpp"
#include <memory>
#include <vector>
#include <map>
#include "Utils/math.hpp"
#include <algorithm>

namespace Zappy {
    class tileInventory : public IScene {
        private:
            TextureManager &_texManager;
            std::unique_ptr<Sprite> _tileInventorySprite;
            Zappy::NetworkManager &_networkManager;
            std::vector<std::unique_ptr<Zappy::IButton>> _buttons;
            std::unique_ptr<Shader> _uiShader;
            int _tileX;
            int _tileY;
        public:
            tileInventory(TextureManager &tm, Zappy::NetworkManager &nm) : _texManager(tm), _networkManager(nm) {}
            void onEnter() override {
                Texture& tileInventoryTex = _texManager.get("gui/assets/tileInventory.png");
                _uiShader = std::make_unique<Shader>("gui/src/Core/Shader/ui.vert", "gui/src/Core/Shader/ui.frag");
                _tileInventorySprite = std::make_unique<Sprite>(tileInventoryTex);
                _tileInventorySprite->setPosition(960.0f, 860.0f);
            }
            SceneState update(const std::vector<Zappy::Event> &events, const Zappy::GameState &gameState, const std::vector<Zappy::NetworkEvent> &netEvents, float deltaTime) override 
            {
                return SceneState::NONE;
            }
            void draw(Shader &shader) override {
                if (!_tileInventorySprite || !_uiShader)
                    return;
                glDisable(GL_DEPTH_TEST);
                _uiShader->bind();
                Zappy::Math::mat4 orthoProjection = Zappy::Math::ortho(0.0f, WIDTH, HEIGHT, 0.0f, -1.0f, 1.0f);
                Zappy::Math::mat4 view;
                _tileInventorySprite->draw(*_uiShader, view, orthoProjection);
                glEnable(GL_DEPTH_TEST);
            }
            void onExit() override {
                _tileInventorySprite.reset();
                _uiShader.reset();
            }
    };
}