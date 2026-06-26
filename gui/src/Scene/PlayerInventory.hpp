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
    class playerInventory : public IScene {
        private:
            TextureManager &_texManager;
            std::unique_ptr<Sprite> _playerInventorySprite;
            Zappy::NetworkManager &_networkManager;
            FontManager &_fontManager;
            std::vector<std::unique_ptr<Zappy::IButton>> _buttons;
            std::unique_ptr<Shader> _uiShader;
            std::unique_ptr<Shader> _textShader;
            std::vector<std::unique_ptr<Sprite>> _resourcesIcons;
            std::vector<std::unique_ptr<Text>> _resourcesTexts;
            std::unique_ptr<Text> _Team;
            std::unique_ptr<Text> _Player;
            std::unique_ptr<Text> _Lvl;
            int _targetPlayerId = -1;
            int _currentLvl = -1;
            std::array<int, 7> _currentQuantity = {-1, -1, -1, -1, -1, -1, -1};
            std::optional<std::reference_wrapper<const Zappy::Tile>> _target;
        public:
            playerInventory(TextureManager &tm, Zappy::NetworkManager &nm, FontManager &fm) : _texManager(tm), _networkManager(nm), _fontManager(fm) {}
            void onEnter() override {
                Texture& playerInventoryTex = _texManager.get("gui/assets/tileInventory.png");
                Font& font = _fontManager.get("gui/assets/fonts/mainTitle.otf", 48.0f);
                Font& infoFont = _fontManager.get("gui/assets/fonts/mainTitle.otf", 32.0f);
                _uiShader = std::make_unique<Shader>("gui/src/Core/Shader/ui.vert", "gui/src/Core/Shader/ui.frag");
                _textShader = std::make_unique<Shader>("gui/src/Core/Shader/text.vert", "gui/src/Core/Shader/text.frag");
                _playerInventorySprite = std::make_unique<Sprite>(playerInventoryTex);
                _playerInventorySprite->isBillboard = false;
                _playerInventorySprite->setPosition(1720.0f, 140.0f);
                _playerInventorySprite->setScale(Zappy::Math::vec3(400.0f, 800.0f, 1.0f));
                _playerInventorySprite->rotation = Zappy::Math::vec3(0.0f, 0.0f, 0.0f);
                float fixedX = 1720.0f - 40.0f;
                float startY = 320.0f;
                float spacing = 80.0f;
                _Player = std::make_unique<Text>(infoFont, "...", 1720 - 100.0f, 200.0f);
                _Player->color = Zappy::Math::vec3(0.0f, 0.0f, 0.0f);
                _Team = std::make_unique<Text>(infoFont, "...", 1720 - 100.0f, 250.0f);
                _Team->color = Zappy::Math::vec3(0.0f, 0.0f, 0.0f);
                _Lvl = std::make_unique<Text>(infoFont, "...", 1720 - 100.0f, 300.0f);
                _Lvl->color = Zappy::Math::vec3(0.0f, 0.0f, 0.0f);
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
            void setTargetPlayer(int id) 
            {
                _targetPlayerId = id;
                _networkManager.sendCommand("pin " + std::to_string(id) + "\n");
                _networkManager.sendCommand("plv " + std::to_string(id) + "\n");
                _currentLvl = -1;
                _currentQuantity.fill(-1);
            }
            SceneState update(const std::vector<Zappy::Event> &events, const Zappy::GameState &gameState, const std::vector<Zappy::NetworkEvent> &netEvents, float deltaTime) override 
            {
                if (_targetPlayerId == -1 || !gameState.players.contains(_targetPlayerId)) {
                    _targetPlayerId = -1; 
                    return SceneState::NONE;
                }
                const Zappy::Player& player = gameState.players.at(_targetPlayerId);

                if (_currentLvl != player.level) {
                    _currentLvl = player.level;
                    std::string playerString = "Player : " + std::to_string(player.id) + "\n";
                    _Player->setString(playerString);
                    std::string playerTeam = "Team : " + player.team + "\n";
                    _Team->setString(playerTeam);
                    std::string playerLvl = "Level : " + std::to_string(player.level) + "\n";
                    _Lvl->setString(playerLvl);
                }
                for (int i = 0; i < 7; ++i) {
                    int amount = player.inventory[i];
                    if (_currentQuantity[i] != amount) {
                        _currentQuantity[i] = amount;
                        if ((size_t) i < _resourcesTexts.size() && _resourcesTexts[i]) {
                            _resourcesTexts[i]->setString(std::to_string(amount));
                        }
                    }
                }
                return SceneState::NONE;
            }
            void draw(Shader &shader,  WindowSize &windowSize) override {
                if (_targetPlayerId == -1 || !_playerInventorySprite || !_uiShader || !_textShader)
                    return;
                glDisable(GL_DEPTH_TEST);
                _uiShader->bind();
                Zappy::Math::mat4 orthoProjection = Zappy::Math::ortho(0.0f, WIDTH, HEIGHT, 0.0f, -1.0f, 1.0f);
                Zappy::Math::mat4 view;
                _playerInventorySprite->draw(*_uiShader, view, orthoProjection);
                for (auto &sprite : _resourcesIcons) {
                    sprite->draw(*_uiShader, view, orthoProjection);
                }
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                if (_Player)
                    _Player->draw(*_textShader, orthoProjection);
                if (_Team)
                    _Team->draw(*_textShader, orthoProjection);
                if (_Lvl)
                    _Lvl->draw(*_textShader, orthoProjection);
                for (auto &txt : _resourcesTexts) {
                    txt->draw(*_textShader, orthoProjection);
                }
                glDisable(GL_BLEND);
                glEnable(GL_DEPTH_TEST);
            }
            void onExit() override {
                _playerInventorySprite.reset();
                _uiShader.reset();
                _Player.reset();
                _Team.reset();
                _Lvl.reset();
                _textShader.reset();
                _resourcesTexts.clear();
                _resourcesIcons.clear();
                _target = std::nullopt;
                _targetPlayerId = -1;
                _currentLvl = -1;
                _currentQuantity.fill(-1);
            }
    };
}