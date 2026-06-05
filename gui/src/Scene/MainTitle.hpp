#pragma once
#include "IScene/IScene.hpp"
#include "Logger.hpp"
#include "Sprite/InstancedGrid.hpp"
#include "Sprite/Sprite.hpp"
#include "Text/Text.hpp"
#include "Font/Font.hpp"
#include "Font/FontManager.hpp"
#include "Texture/TextureManager.hpp"
#include "Utils/math.hpp"

#include "Render/Camera.hpp"
#include "Render/Render.hpp"
#include "Audio/audio.hpp"
#include <memory>
#include <vector>

namespace Zappy {

    class MainTitle : public IScene {
        private:
            TextureManager &_texManager;
            std::unique_ptr<Renderer> _renderer;
            Camera _camera;
            std::unique_ptr<InstancedGrid> _floor;
            std::vector<std::unique_ptr<Sprite>> _players;
            float _animationTime = 0.0f;
            bool _zoomFinished = false;
            float _zoomDuration = 4.0f;
            float _textFadeTime = 0.0f;
            float _textFadeDuration = 2.0f;
            Zappy::Audio _audio;
            FontManager _fontManager;
            std::unique_ptr<Shader> _textShader;
            std::unique_ptr<Text> _titleText;
            std::unique_ptr<Text> _pressStartText;
        public:
            MainTitle(TextureManager &tm) : _texManager(tm) {}
            void onEnter() override {
                _textShader = std::make_unique<Shader>("gui/src/Core/Shader/text.vert", "gui/src/Core/Shader/text.frag");
                Font &titleFont = _fontManager.get("gui/assets/fonts/mainTitle.otf", 256.0f, 4096);
                _titleText = std::make_unique<Text>(titleFont, "ZAPPY", 0.0f, 100.0f);
                _titleText->color = Zappy::Math::vec3(1.0f, 0.8f, 0.0f);
                float titleX = (WIDTH / 2.0f) - (_titleText->getWidth() / 2.0f);
                _titleText->setPosition(titleX, 200.0f);
                _titleText->alpha = 0.0f;

                Font &startFont = _fontManager.get("gui/assets/fonts/mainTitle.otf", 64.0f, 1024);
                _pressStartText = std::make_unique<Text>(startFont, "Press ANY KEY to start", 0.0f, 200.0f);
                _pressStartText->color = Zappy::Math::vec3(1.0f, 1.0f, 1.0f);
                float pressX = (WIDTH / 2.0f) - (_pressStartText->getWidth() / 2.0f);
                _pressStartText->setPosition(pressX, 1000.0f);
                _pressStartText->alpha = 0.0f;
                _audio.playMusic("gui/assets/musics/MainTitle.mp3");
                _renderer = std::make_unique<Renderer>(WIDTH, HEIGHT);
                Texture &islandTex = _texManager.get("gui/assets/island.png");
                Texture &cuteTex = _texManager.get("gui/assets/cute.png");
                _floor = std::make_unique<InstancedGrid>(islandTex);

                for (int x = -5; x <= 5; x++) {
                    for (int z = -5; z <= 5; z++) {
                        _floor->addTile(Zappy::Math::vec3(x * 2.0f, 0.0f, z * 2.0f), 2.0f);

                        if ((x + z) & 1) {
                            auto player = std::make_unique<Sprite>(cuteTex);
                            player->position = Zappy::Math::vec3(x * 2.0f, 0.0f, z * 2.0f - 1.0f);
                            player->scale = Zappy::Math::vec3(1.0f, 1.0f, 1.0f);
                            player->isBillboard = true;
                            _players.push_back(std::move(player));
                        }
                    }
                }
                _floor->build();
                _camera.position = Zappy::Math::vec3(0.0f, 100.0f, 0.0f);
                _camera.pitch = -89.0f;
            }
            SceneState update(const std::vector<Zappy::Event> &events, float deltaTime) override {
                if (!_zoomFinished) {
                    _animationTime += deltaTime;
                    float t = std::min(_animationTime / _zoomDuration, 1.0f);
                    float easeT = t * t * (3.0f - 2.0f * t);
                    Zappy::Math::vec3 startPos(0.0f, 100.0f, 10.0f);
                    Zappy::Math::vec3 endPos(0.0f, 25.0f, 35.0f);
                    _camera.position = Zappy::Math::transi(startPos, endPos, easeT);
                    _camera.pitch = Zappy::Math::transi(-89.0f, -35.0f, easeT);
                    
                    if (t >= 1.0f) {
                        _zoomFinished = true;
                    }
                } else {
                    if (_textFadeTime < _textFadeDuration) {
                        _textFadeTime += deltaTime;
                        float alphaT = std::min(_textFadeTime / _textFadeDuration, 1.0f);
                        _titleText->alpha = alphaT;
                        float pressStartAlphaT = std::max(0.0f, std::min((_textFadeTime - (_textFadeDuration / 2.0f)) / (_textFadeDuration / 2.0f), 1.0f));
                        _pressStartText->alpha = pressStartAlphaT;
                    }
                    for (const auto &event : events) {
                        if (event.type == Zappy::EventType::KeyPressed || event.type == Zappy::EventType::MousePressed) {
                            return SceneState::MENU;
                        }
                    }
                }
                return SceneState::NONE;
            }
            void draw(Shader &) override {
                glEnable(GL_DEPTH_TEST);
                if (_renderer && _floor) {
                    _renderer->render(_camera, *_floor, _players);
                }
                glDisable(GL_DEPTH_TEST);
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                Zappy::Math::mat4 orthoProjection = Zappy::Math::ortho(0.0f, WIDTH, HEIGHT, 0.0f, -1.0f, 1.0f);
                if (_zoomFinished) {
                    _titleText->draw(*_textShader, orthoProjection);
                    _pressStartText->draw(*_textShader, orthoProjection);
                }
                glDisable(GL_BLEND);
                glEnable(GL_DEPTH_TEST);
            }
            void onExit() override {
                _players.clear();
                _floor.reset();
                _renderer.reset();
                _audio.stopMusic();
            }
    };
}