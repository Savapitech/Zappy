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

  float _animationTime;

public:
  LoadNetwork(TextureManager &tm, NetworkManager &net) 
      : _texManager(tm), _network(net), _animationTime(0.0f) {}

  void onEnter() override {
    LOG_INFO("Entering LoadNetwork Scene...");
    _renderer = std::make_unique<Renderer>(WIDTH, HEIGHT);
    _animationTime = 0.0f;
  }

  SceneState update(const std::vector<Zappy::Event> &events,
                    const Zappy::GameState &gameState,
                    const std::vector<Zappy::NetworkEvent> &netEvents,
                    float deltaTime) override 
  {
    _animationTime += deltaTime;

    return SceneState::MENU;
  }

  void draw(Shader &shader) override {
    if (_renderer) {
    }
  }

  void onExit() override {
    LOG_INFO("Exiting LoadNetwork Scene...");
    _renderer.reset();
  }
};
} // namespace Zappy