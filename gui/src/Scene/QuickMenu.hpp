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
    class quickMenu : public IScene {
        private:
            TextureManager &_texManager;
            std::unique_ptr<Sprite> _backgroundSprite;
            Zappy::NetworkManager &_networkManager;
            std::vector<std::unique_ptr<Zappy::IButton>> _buttons;
            std::unique_ptr<Shader> _uiShader;
            int _speed;
        public:
            quickMenu(TextureManager &tm, Zappy::NetworkManager &nm) : _texManager(tm), _networkManager(nm) {
                _speed = 0;
            }
            void onEnter() override {
                Texture& menuTex = _texManager.get("gui/assets/quickMenu.png");
                Texture& speedButtonTex = _texManager.get("gui/assets/speedButton.png");
                Texture& decreaseButtonTex = _texManager.get("gui/assets/minus.png");
                Texture& increaseButtonTex = _texManager.get("gui/assets/plus.png");
                _uiShader = std::make_unique<Shader>("gui/src/Core/Shader/ui.vert", "gui/src/Core/Shader/ui.frag");
                _backgroundSprite = std::make_unique<Sprite>(menuTex);
                _backgroundSprite->setPosition(960.0f, 332.5f);
                _networkManager.sendCommand("sgt\n");
                auto helper = []() {
                    std::cout << "You can decrease / increase the number of ticks per seconds." << std::endl;
                };
                auto decrease = [this]() {
                    int newSpeed = std::max(1, this->_speed - 5);
                    this->_networkManager.sendCommand("sst " + std::to_string(newSpeed) + "\n");
                    std::cout << "Decrease requested: " << newSpeed << std::endl;
                };
                auto increase = [this]() {
                    int newSpeed = this->_speed + 5;
                    this->_networkManager.sendCommand("sst " + std::to_string(newSpeed) + "\n");
                    std::cout << "Increase requested: " << newSpeed << std::endl;
                };
                _buttons.push_back(std::make_unique<Zappy::Button>(speedButtonTex, 950.0f, 450.0f, 80.0f, 80.0f, helper));
                _buttons.push_back(std::make_unique<Zappy::Button>(increaseButtonTex, 1050.0f, 465.0f, 50.0f, 50.0f, increase));
                _buttons.push_back(std::make_unique<Zappy::Button>(decreaseButtonTex, 850.0f, 465.0f, 50.0f, 50.0f, decrease));
                _backgroundSprite->scale = Zappy::Math::vec3(415.0f, 415.0f, 1.0f);
            }
            SceneState update(const std::vector<Zappy::Event> &events, const Zappy::GameState &gameState, const std::vector<Zappy::NetworkEvent> &netEvents, float deltaTime) override 
            {
                _speed = gameState.map.timeUnit;
                for (auto& button : _buttons) {
                    button->update(events);
                }
                return SceneState::NONE;
            }
            void draw(Shader &shader, WindowSize &windowSize) override {
                if (!_backgroundSprite || !_uiShader)
                    return;
                glDisable(GL_DEPTH_TEST);
                _uiShader->bind();
                Zappy::Math::mat4 orthoProjection = Zappy::Math::ortho(0.0f, WIDTH, HEIGHT, 0.0f, -1.0f, 1.0f);
                Zappy::Math::mat4 view;
                _backgroundSprite->draw(*_uiShader, view, orthoProjection);
                for (auto &button : _buttons) {
                    button->draw(*_uiShader);
                }
                glEnable(GL_DEPTH_TEST);
            }
            void onExit() override {
                _buttons.clear();
                _backgroundSprite.reset();
                _uiShader.reset();
            }
    };
}
