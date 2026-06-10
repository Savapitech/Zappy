#pragma once

#include "IScene/IScene.hpp"
#include "Logger.hpp"
#include "Texture/TextureManager.hpp"
#include "Network/NetworkManager.hpp"
#include "Render/Camera.hpp"
#include "Render/Render.hpp"

#include <memory>
#include <vector>
#include <functional>

namespace Zappy {
class LoadNetwork : public IScene {
private:
  TextureManager &_texManager;
  std::reference_wrapper<NetworkManager> _network;
  std::unique_ptr<Renderer> _renderer;
  Camera _camera;

  std::unique_ptr<Sprite> _loadingSprite;
  float _animationTime;
  float _retryTimer;

public:
  LoadNetwork(TextureManager &tm, NetworkManager &net) 
      : _texManager(tm), _network(net), _animationTime(0.0f), _retryTimer(0.0f) {}

  void onEnter() override {
    LOG_INFO("Entering LoadNetwork Scene...");

    Texture &loadingAsset = _texManager.get("gui/assets/loadingNetworkSheat.png");

    _loadingSprite = std::make_unique<Sprite>(loadingAsset);
    _renderer = std::make_unique<Renderer>(WIDTH, HEIGHT);
    _animationTime = 0.0f;
    _retryTimer = 0.0f;
  }

  SceneState update(const std::vector<Zappy::Event> &events,
                    const Zappy::GameState &gameState,
                    const std::vector<Zappy::NetworkEvent> &netEvents,
                    float deltaTime) override 
  {
    _animationTime += deltaTime;
    _retryTimer -= deltaTime;

    if (_retryTimer <= 0.0f) {
        LOG_INFO("Attempting to connect to server...");
        
        if (_network.get().connectToServer("127.0.0.1", 4242) == true) {
            LOG_INFO("Network success");
            return SceneState::MENU;
        } else {
            LOG_INFO("Network failed, retrying in 2 seconds...");
            _retryTimer = 2.0f;
        }
    }
    
    return SceneState::NONE;
  }

void draw(Shader &shader) override {
    _renderer->drawSkybox(_camera);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Zappy::Math::mat4 orthoProj = Zappy::Math::ortho(0.0f, WIDTH, HEIGHT, 0.0f, -1.0f, 1.0f);
    Zappy::Math::mat4 identityView;

    shader.bind();

    int cols = 4;
    int rows = 3;
    int totalFrames = 12;
    float fps = 15.0f;

    int currentFrame = static_cast<int>(_animationTime * fps) % totalFrames;

    int col = currentFrame % cols;
    int row = (rows - 1) - (currentFrame / cols); 

    float scaleX = 1.0f / cols;
    float scaleY = 1.0f / rows;

    _loadingSprite->_uvScale.x = scaleX;
    _loadingSprite->_uvScale.y = scaleY;
    _loadingSprite->_uvOffset.x = col * scaleX;
    _loadingSprite->_uvOffset.y = row * scaleY;

    _loadingSprite->position = Zappy::Math::vec3(WIDTH / 2.0f, HEIGHT / 2.0f, 0.0f);
    _loadingSprite->scale = Zappy::Math::vec3(130.0f, 120.0f, 1.0f); 

    _loadingSprite->draw(shader, identityView, orthoProj);

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
  }
  void onExit() override {
    LOG_INFO("Exiting LoadNetwork Scene...");
    _renderer.reset();
  }
};
} // namespace Zappy