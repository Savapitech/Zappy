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
#include "Audio/audio.hpp"
#include <memory>
#include <vector>

namespace Zappy
{
    class IntroScene : public IScene {
        private:
            TextureManager &_texManager;
            std::unique_ptr<Renderer> _renderer;
            Zappy::Audio _audio;
            std::unique_ptr<Shader> _fadeShader;
            std::unique_ptr<Sprite> _studioSprite;
            unsigned int _emptyVAO = 0;
            float _blackScreenDuration = 1.0f;
            float _studioNameDuration = 2.0f;
            bool _isFinished = false;
            float _fadeInDuration = 1.5f;
            float _animationTime = 0.0f;
            float _fadeOutDuration = 2.0f;
            float _currentAlpha = 0.0f;
        public:
            IntroScene(TextureManager &tm) : _texManager(tm) {}
            void onEnter() override {
                _fadeShader = std::make_unique<Shader>("gui/src/Core/Shader/fade.vert", "gui/src/Core/Shader/fade.frag");
                glGenVertexArrays(1, &_emptyVAO);
                _audio.playMusic("gui/assets/musics/MainTitle.mp3");
                _renderer = std::make_unique<Renderer>(WIDTH, HEIGHT);
                Texture &studioName = _texManager.get("gui/assets/StudioName.png");
                _studioSprite = std::make_unique<Sprite>(studioName);
                _studioSprite->setPosition(WIDTH / 2.0f, HEIGHT / 2.0f);
            }
            SceneState update(const std::vector<Zappy::Event> &events, 
                            const Zappy::GameState &gameState,
                            const std::vector<Zappy::NetworkEvent> &netEvents,
                            float deltaTime) override {
                if (!_isFinished) {
                    _animationTime += deltaTime;
                    float fadeInEnd = _blackScreenDuration + _fadeInDuration;
                    float visibleEnd = fadeInEnd + _studioNameDuration;
                    float fadeOutEnd = visibleEnd + _fadeOutDuration;
                    if (_animationTime < _blackScreenDuration){
                        _currentAlpha = 0.0f;
                    }
                    else if (_animationTime < fadeInEnd) {
                        _currentAlpha = (_animationTime - _blackScreenDuration) / _fadeInDuration;
                    }
                    else if (_animationTime < visibleEnd) {
                        _currentAlpha = 1.0f;
                    }
                    else if (_animationTime < fadeOutEnd) {
                        _currentAlpha = 1.0f - ((_animationTime - visibleEnd) / _fadeOutDuration);
                    }
                    else {
                        _isFinished = true;
                        return SceneState::TITLE;
                    }
                }
                return SceneState::INTRO;
            }
            void draw(Shader &shader) override {
                _fadeShader->bind();
                _fadeShader->setFloat("u_alpha", _currentAlpha);
                if (_studioSprite){
                    Zappy::Math::mat4 view;
                    Zappy::Math::mat4 projection = Zappy::Math::ortho(0.0f, WIDTH, HEIGHT, 0.0f, -1.0f, 1.0f);
                    _studioSprite->draw(*_fadeShader, view, projection);
                }
            }
            void onExit() override {
                if (_emptyVAO != 0) {
                    glDeleteVertexArrays(1, &_emptyVAO);
                    _emptyVAO = 0;
                }
            }


    };
}