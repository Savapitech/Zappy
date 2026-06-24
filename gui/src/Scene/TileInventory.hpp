#pragma once

#include "IScene/IScene.hpp"
#include "Texture/TextureManager.hpp"
#include "Sprite/Sprite.hpp"
#include "Logger.hpp"
#include "Buttons/Button.hpp"
#include "Network/NetworkManager.hpp"
#include "Render/Render.hpp"
#include "Text/Text.hpp"
#include "Font/FontManager.hpp"
#include <optional>
#include <functional>
#include <memory>
#include <string>
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
            FontManager &_fontManager;
            std::vector<std::unique_ptr<Zappy::IButton>> _buttons;
            std::unique_ptr<Shader> _uiShader;
            std::unique_ptr<Shader> _textShader;
            std::vector<std::unique_ptr<Sprite>> _resourcesIcons;
            std::vector<std::unique_ptr<Text>> _resourcesTexts;
            std::array<int, 7> _currentQuantity = {-1, -1, -1, -1, -1, -1, -1};
            std::optional<std::reference_wrapper<const Zappy::Tile>> _target;
        public:
            tileInventory(TextureManager &tm, Zappy::NetworkManager &nm, FontManager &fm) : _texManager(tm), _networkManager(nm), _fontManager(fm) {}
            void onEnter() override {
                Texture& tileInventoryTex = _texManager.get("gui/assets/tileInventory.png");
                Font& font = _fontManager.get("gui/assets/fonts/mainTitle.otf", 48.0f);
                _uiShader = std::make_unique<Shader>("gui/src/Core/Shader/ui.vert", "gui/src/Core/Shader/ui.frag");
                _textShader = std::make_unique<Shader>("gui/src/Core/Shader/text.vert", "gui/src/Core/Shader/text.frag");
                _tileInventorySprite = std::make_unique<Sprite>(tileInventoryTex);
                _tileInventorySprite->isBillboard = false;
                _tileInventorySprite->setPosition(200.0f, 140.0f);
                _tileInventorySprite->setScale(Zappy::Math::vec3(400.0f, 800.0f, 1.0f));
                _tileInventorySprite->rotation = Zappy::Math::vec3(0.0f, 0.0f, 0.0f);
                float fixedX = 160.0f;
                float startY = 180.0f;
                float spacing = 110.0f;
                for (int i = 0; i < 7; i++) {
                    Texture &resTex = _texManager.get("gui/assets/resource_" + std::to_string(i) + ".png");
                    auto spr = std::make_unique<Sprite>(resTex);
                    spr->isBillboard = false;
                    spr->rotation = Zappy::Math::vec3(0.0f, 0.0f, 0.0f);
                    spr->setPosition(fixedX, startY + (i * spacing));
                    spr->setScale(Zappy::Math::vec3(50.0f, 50.0f, 1.0f));
                    _resourcesIcons.push_back(std::move(spr));
                    auto txt = std::make_unique<Text>(font, "0", fixedX + 60.0f, startY + (i * spacing) + 40.0f);
                    txt->color = Zappy::Math::vec3(0.0f, 0.0f, 0.0f);
                    _resourcesTexts.push_back(std::move(txt));
                }
            }
            void setTargetTile(const Zappy::Tile& tile) 
            {
                _target = tile;
            }
            SceneState update(const std::vector<Zappy::Event> &events, const Zappy::GameState &gameState, const std::vector<Zappy::NetworkEvent> &netEvents, float deltaTime) override 
            {
                if (!_target)
                    return SceneState::NONE;
                const Zappy::Tile& currentTile = _target->get();

                for (int i = 0; i < 7; ++i) {
                    int amount = currentTile.resources[i];
                    if (_currentQuantity[i] != amount) {
                        _currentQuantity[i] = amount;
                        if ((size_t)i < _resourcesTexts.size() && _resourcesTexts[i])
                            _resourcesTexts[i]->setString(std::to_string(amount));
                    }
                }
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
                for (auto &sprite : _resourcesIcons) {
                    sprite->draw(*_uiShader, view, orthoProjection);
                }
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                for (auto &txt : _resourcesTexts) {
                    txt->draw(*_textShader, orthoProjection);
                }
                glDisable(GL_BLEND);
                glEnable(GL_DEPTH_TEST);
            }
            void onExit() override {
                _tileInventorySprite.reset();
                _uiShader.reset();
                _textShader.reset();
                _resourcesTexts.clear();
                _resourcesIcons.clear();
                _target = std::nullopt;
                _currentQuantity.fill(-1);
            }
    };
}