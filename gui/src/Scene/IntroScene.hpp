#pragma once
#include "Audio/audioManager.hpp"
#include "Font/Font.hpp"
#include "Font/FontManager.hpp"
#include "IScene/IScene.hpp"
#include "Logger.hpp"
#include "Render/Camera.hpp"
#include "Render/Render.hpp"
#include "Sprite/InstancedGrid.hpp"
#include "Sprite/Sprite.hpp"
#include "Text/Text.hpp"
#include "Texture/TextureManager.hpp"
#include "Utils/math.hpp"
#include <cmath>
#include <cstdlib>
#include <memory>
#include <vector>

namespace Zappy {
class IntroScene : public IScene {
private:
  struct Star {
    Zappy::Math::vec2 position;
    float baseSize;
    float phase;
    float speed;
  };
  std::vector<Star> _stars;
  unsigned int _starVAO = 0;
  unsigned int _starVBO = 0;
  std::unique_ptr<Shader> _particleShader;
  float randFloat(float min, float max) {
    return min + static_cast<float>(rand()) /
                     (static_cast<float>(RAND_MAX) / (max - min));
  }
  TextureManager &_texManager;

  std::unique_ptr<Renderer> _renderer;
  Zappy::audioManager &_audios;
  std::unique_ptr<Shader> _fadeShader;
  std::unique_ptr<Shader> _spriteShader;
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
  IntroScene(TextureManager &tm, audioManager &am)
      : _texManager(tm), _audios(am) {
    _audios.upload("gui/assets/musics/MainTitle.mp3");
  }
  void onEnter() override {
    _fadeShader = std::make_unique<Shader>("gui/src/Core/Shader/fade.vert",
                                           "gui/src/Core/Shader/fade.frag");
    _spriteShader =
        std::make_unique<Shader>("gui/src/Core/Shader/spriteFade.vert",
                                 "gui/src/Core/Shader/spriteFade.frag");
    glGenVertexArrays(1, &_emptyVAO);
    _audios.playMusicAt("gui/assets/musics/MainTitle.mp3");
    _renderer = std::make_unique<Renderer>(WIDTH, HEIGHT);
    Texture &studioName = _texManager.get("gui/assets/StudioName.png");
    _studioSprite = std::make_unique<Sprite>(studioName);
    _studioSprite->setPosition(WIDTH / 2.0f, HEIGHT / 4.0f);
    _studioSprite->scale = Zappy::Math::vec3(500.0f, 500.0f, 1.0f);
    _particleShader =
        std::make_unique<Shader>("gui/src/Core/Shader/particle.vert",
                                 "gui/src/Core/Shader/particle.frag");
    for (int i = 0; i < 150; i++) {
      Star s;
      s.position =
          Zappy::Math::vec2(randFloat(0.0f, WIDTH), randFloat(0.0f, HEIGHT));
      s.baseSize = randFloat(1.5f, 4.0f);
      s.phase = randFloat(0.0f, 6.28f);
      s.speed = randFloat(1.0f, 4.0f);
      _stars.push_back(s);
    }
    glGenVertexArrays(1, &_starVAO);
    glGenBuffers(1, &_starVBO);
    glBindVertexArray(_starVAO);
    glBindBuffer(GL_ARRAY_BUFFER, _starVBO);
    glBufferData(GL_ARRAY_BUFFER, _stars.size() * 6 * sizeof(float), nullptr,
                 GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                          (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                          (void *)(2 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                          (void *)(5 * sizeof(float)));
    glBindVertexArray(0);
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
      if (_animationTime < _blackScreenDuration) {
        _currentAlpha = 0.0f;
      } else if (_animationTime < fadeInEnd) {
        _currentAlpha =
            (_animationTime - _blackScreenDuration) / _fadeInDuration;
      } else if (_animationTime < visibleEnd) {
        _currentAlpha = 1.0f;
      } else if (_animationTime < fadeOutEnd) {
        _currentAlpha =
            1.0f - ((_animationTime - visibleEnd) / _fadeOutDuration);
      } else {
        _isFinished = true;
        return SceneState::TITLE;
      }
    }
    return SceneState::NONE;
  }
  void draw(Shader &shader) override {
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    _fadeShader->bind();
    _fadeShader->setFloat("u_Alpha", 1.0f);
    glBindVertexArray(_emptyVAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
    glEnable(GL_PROGRAM_POINT_SIZE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    _particleShader->bind();
    Zappy::Math::mat4 projection =
        Zappy::Math::ortho(0.0f, WIDTH, HEIGHT, 0.0f, -1.0f, 1.0f);
    _particleShader->setMat4("u_Projection", projection);
    _particleShader->setFloat("u_Alpha", 1.0f);
    std::vector<float> pData;
    pData.reserve(_stars.size() * 6);
    for (const auto &s : _stars) {
      float twinkle =
          (std::sin(_animationTime * s.speed + s.phase) + 1.0f) / 2.0f;
      pData.push_back(s.position.x);
      pData.push_back(s.position.y);
      pData.push_back(twinkle);
      pData.push_back(twinkle);
      pData.push_back(twinkle);
      pData.push_back(s.baseSize);
    }
    glBindVertexArray(_starVAO);
    glBindBuffer(GL_ARRAY_BUFFER, _starVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, pData.size() * sizeof(float),
                    pData.data());
    glDrawArrays(GL_POINTS, 0, _stars.size());
    glBindVertexArray(0);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    if (_studioSprite) {
      _spriteShader->bind();
      _spriteShader->setFloat("u_Alpha", _currentAlpha);
      Zappy::Math::mat4 view;
      Zappy::Math::mat4 projection =
          Zappy::Math::ortho(0.0f, WIDTH, HEIGHT, 0.0f, -1.0f, 1.0f);
      _studioSprite->draw(*_spriteShader, view, projection);
    }
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
  }
  void onExit() override {
    if (_emptyVAO != 0) {
      glDeleteVertexArrays(1, &_emptyVAO);
      _emptyVAO = 0;
    }
    if (_starVAO != 0) {
      glDeleteVertexArrays(1, &_starVAO);
      _starVAO = 0;
    }
    if (_starVBO != 0) {
      glDeleteBuffers(1, &_starVBO);
      _starVBO = 0;
    }
  }
};
} // namespace Zappy
