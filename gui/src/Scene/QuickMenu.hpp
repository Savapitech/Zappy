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
            std::unique_ptr<Zappy::IButton> _SpeedButton;
            std::unique_ptr<Zappy::IButton> _IncreaseButton;
            std::unique_ptr<Zappy::IButton> _DecreaseButton;
            std::unique_ptr<Shader> _uiShader;
            int _speed;
            WindowSize &_windowSize;
        public:
            quickMenu(TextureManager &tm, Zappy::NetworkManager &nm, WindowSize &ws) : _texManager(tm), _networkManager(nm), _windowSize(ws) {
                _speed = 0;
            }
            void onEnter() override {
                Texture& menuTex = _texManager.get("gui/assets/quickMenu.png");
                Texture& speedButtonTex = _texManager.get("gui/assets/speedButton.png");
                Texture& decreaseButtonTex = _texManager.get("gui/assets/minus.png");
                Texture& increaseButtonTex = _texManager.get("gui/assets/plus.png");
                _uiShader = std::make_unique<Shader>("gui/src/Core/Shader/ui.vert", "gui/src/Core/Shader/ui.frag");
                _backgroundSprite = std::make_unique<Sprite>(menuTex);
                
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
                _SpeedButton = std::make_unique<Zappy::Button>(speedButtonTex, (_windowSize.width / 2.0f) - 40.0f, (_windowSize.height / 2.0f) - 40.0f, 80.0f, 80.0f, helper, _windowSize);
                _IncreaseButton = std::make_unique<Zappy::Button>(increaseButtonTex, 1050.0f, 465.0f, 50.0f, 50.0f, increase, _windowSize);
                _DecreaseButton = std::make_unique<Zappy::Button>(decreaseButtonTex, 850.0f, 465.0f, 50.0f, 50.0f, decrease, _windowSize);
                _backgroundSprite->scale = Zappy::Math::vec3(415.0f, 415.0f, 1.0f);
            }
            SceneState update(const std::vector<Zappy::Event> &events, const Zappy::GameState &gameState, const std::vector<Zappy::NetworkEvent> &netEvents, float deltaTime) override 
            {
                float centerX = _windowSize.width / 2.0f;
                float centerY = _windowSize.height / 2.0f;
                
                _backgroundSprite->setPosition(_windowSize.width / 2.0f, (_windowSize.height - 415.0f) / 2.0f);
                _speed = gameState.map.timeUnit;
                _SpeedButton->setPosition(centerX - 20.0f, centerY - 40.0f);
                _IncreaseButton->setPosition(centerX + 80.0f, centerY - 35.0f);
                _DecreaseButton->setPosition(centerX - 95.0f, centerY - 35.0f);
                _SpeedButton->update(events);
                _DecreaseButton->update(events);
                _IncreaseButton->update(events);
                return SceneState::NONE;
            }
            void draw(Shader &shader, WindowSize &windowSize) override {
                if (!_backgroundSprite || !_uiShader)
                    return;
                glDisable(GL_DEPTH_TEST);
                _uiShader->bind();
                Zappy::Math::mat4 orthoProjection = Zappy::Math::ortho(0.0f, _windowSize.width, _windowSize.height, 0.0f, -1.0f, 1.0f);
                Zappy::Math::mat4 view;
                _backgroundSprite->draw(*_uiShader, view, orthoProjection);
                _SpeedButton->draw(*_uiShader);
                _IncreaseButton->draw(*_uiShader);
                _DecreaseButton->draw(*_uiShader);
                glEnable(GL_DEPTH_TEST);
            }
            void onExit() override {
                _SpeedButton.reset();
                _backgroundSprite.reset();
                _uiShader.reset();
            }
    };
}
