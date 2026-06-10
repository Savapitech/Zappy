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
#include <cstdlib>
#include <cmath>
#include "Render/Camera.hpp"
#include "Render/Render.hpp"
#include "Audio/audioManager.hpp"
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
            Zappy::audioManager &_audios;
            FontManager _fontManager;
            std::unique_ptr<Shader> _textShader;
            std::unique_ptr<Text> _titleText;
            std::unique_ptr<Text> _pressStartText;
            struct Particle {
                Zappy::Math::vec2 position;
                Zappy::Math::vec2 velocity;
                Zappy::Math::vec3 color;
                float size;
            };
            std::vector<Particle> _particles;
            unsigned int _particleVAO = 0;
            unsigned int _particleVBO = 0;
            std::unique_ptr<Shader> _particleShader;
            float randFloat(float min, float max)
            {
                float random = min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
                return random;
            }
            void fadeInOut(Text &text, float time, float speed = 3.0f) 
            {
                text.alpha = (std::sin(time * speed) + 1.0f) / 2.0f;
            }
            bool _isExiting = false;
            float _exitFadeTime = 0.0f;
            float _exitFadeDuration = 1.0f;
            std::unique_ptr<Shader> _fadeShader;
            unsigned int _emptyVAO = 0;

        public:
            MainTitle(TextureManager &tm, audioManager &audioManager) : _texManager(tm), _audios(audioManager) {}
            void onEnter() override 
            {
                _isExiting = false;
                _exitFadeTime = 0.0f;        
                _fadeShader = std::make_unique<Shader>("gui/src/Core/Shader/fade.vert", "gui/src/Core/Shader/fade.frag");
                glGenVertexArrays(1, &_emptyVAO);
                _textShader = std::make_unique<Shader>("gui/src/Core/Shader/text.vert", "gui/src/Core/Shader/text.frag");
                Font &titleFont = _fontManager.get("gui/assets/fonts/mainTitle.otf", 256.0f, 4096);
                _titleText = std::make_unique<Text>(titleFont, "ZAPPY", 0.0f, 100.0f);
                _titleText->color = Zappy::Math::vec3(1.0f, 0.8f, 0.0f);
                _titleText->scaleRatio = Zappy::Math::vec2(1.0f, 0.55f);
                _titleText->letterSpacing = 3.0f;
                float titleX = (WIDTH / 2.0f) - (_titleText->getWidth() / 2.0f);
                _titleText->setPosition(titleX, 200.0f);
                _titleText->alpha = 0.0f;

                Font &startFont = _fontManager.get("gui/assets/fonts/mainTitle.otf", 64.0f, 1024);
                _pressStartText = std::make_unique<Text>(startFont, "Press ANY KEY to start", 0.0f, 200.0f);
                _pressStartText->color = Zappy::Math::vec3(1.0f, 1.0f, 1.0f);
                float pressX = (WIDTH / 2.0f) - (_pressStartText->getWidth() / 2.0f);
                _pressStartText->setPosition(pressX, 1000.0f);
                _pressStartText->alpha = 0.0f;
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
                _particleShader = std::make_unique<Shader>("gui/src/Core/Shader/particle.vert", "gui/src/Core/Shader/particle.frag");

                for (int i = 0; i < 300; i++) {
                    Particle p;
                    p.position = Zappy::Math::vec2(randFloat(0.0f, WIDTH), randFloat(0.0f, HEIGHT));
                    p.velocity = Zappy::Math::vec2(0.0f, randFloat(30.0f, 100.0f));
                    p.color = Zappy::Math::vec3(1.0f, 0.8f, 0.0f);
                    p.size = randFloat(2.0f, 6.0f);
                    _particles.push_back(p);
                }
                glGenVertexArrays(1, &_particleVAO);
                glGenBuffers(1, &_particleVBO);
                glBindVertexArray(_particleVAO);
                glBindBuffer(GL_ARRAY_BUFFER, _particleVBO);
                glBufferData(GL_ARRAY_BUFFER, _particles.size() * 6 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    
                glEnableVertexAttribArray(0);
                glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
                glEnableVertexAttribArray(1);
                glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(2 * sizeof(float)));
                glEnableVertexAttribArray(2);
                glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(5 * sizeof(float)));
                glBindVertexArray(0);
            }
            SceneState update(const std::vector<Zappy::Event> &events, 
                            const Zappy::GameState &gameState,
                            const std::vector<Zappy::NetworkEvent> &netEvents,
                            float deltaTime) override {
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
                    _textFadeTime += deltaTime;
                    _titleText->alpha = std::min(_textFadeTime / _textFadeDuration, 1.0f);
                    if (_textFadeTime > (_textFadeDuration / 2.0f))
                        fadeInOut(*_pressStartText, _textFadeTime, 2.0f);
                    else 
                        _pressStartText->alpha = 0.0f;
                    
                    for (auto &p : _particles) {
                        p.position.y += p.velocity.y * deltaTime;
                        if (p.position.y > HEIGHT) {
                            p.position.y = -10.0f;
                            p.position.x = randFloat(0.0f, WIDTH);
                        }
                    }
                    if (!_isExiting) {
                        for (const auto &event : events) {
                            if (event.type == Zappy::EventType::KeyPressed || event.type == Zappy::EventType::MousePressed) {
                                _isExiting = true;
                                break;
                            }
                        }
                    } else {
                        _exitFadeTime += deltaTime;
                        if (_exitFadeTime >= _exitFadeDuration) {
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
                    glEnable(GL_PROGRAM_POINT_SIZE);
                    glEnable(GL_BLEND);
                    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
                    _particleShader->bind();
                    _particleShader->setMat4("u_Projection", orthoProjection);
                    float alphaT = std::min(_textFadeTime / _textFadeDuration, 1.0f);
                    _particleShader->setFloat("u_Alpha", alphaT);
                    std::vector<float> pData;
                    pData.reserve(_particles.size() * 6);
                    for (const auto &p : _particles) {
                        pData.push_back(p.position.x); pData.push_back(p.position.y);
                        pData.push_back(p.color.x); pData.push_back(p.color.y); pData.push_back(p.color.z);
                        pData.push_back(p.size);
                    }
                    glBindVertexArray(_particleVAO);
                    glBindBuffer(GL_ARRAY_BUFFER, _particleVBO);
                    glBufferSubData(GL_ARRAY_BUFFER, 0, pData.size() * sizeof(float), pData.data());
                    glDrawArrays(GL_POINTS, 0, _particles.size());
                    glBindVertexArray(0);
                    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                    _titleText->draw(*_textShader, orthoProjection);
                    _pressStartText->draw(*_textShader, orthoProjection);
                }
                glDisable(GL_BLEND);
                glEnable(GL_DEPTH_TEST);
                if (_isExiting) {
                    glEnable(GL_BLEND);
                    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                    glDisable(GL_DEPTH_TEST);
                    _fadeShader->bind();
                    float fadeAlpha = std::min(_exitFadeTime / _exitFadeDuration, 1.0f);
                    _fadeShader->setFloat("u_Alpha", fadeAlpha);
                    glBindVertexArray(_emptyVAO);
                    glDrawArrays(GL_TRIANGLES, 0, 3);
                    glBindVertexArray(0);        
                    glEnable(GL_DEPTH_TEST);
                }
            }
            void onExit() override {
                _players.clear();
                _floor.reset();
                _renderer.reset();
                _audios.stopMusicAt("gui/assets/musics/MainTitle.mp3");
                if (_particleVAO != 0) 
                    glDeleteVertexArrays(1, &_particleVAO);
                if (_particleVBO != 0) 
                    glDeleteBuffers(1, &_particleVBO);
                _particles.clear();
                if (_emptyVAO != 0)
                    glDeleteVertexArrays(1, &_emptyVAO);
            }
    };
}
