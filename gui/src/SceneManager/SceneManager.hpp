#pragma once

#include "IScene/IScene.hpp"
#include "Scene/Menu.hpp"
#include "Scene/MainTitle.hpp"
#include "Texture/TextureManager.hpp"
#include "Network/NetworkManager.hpp"
#include <memory>
#include <vector>

namespace Zappy {
class SceneManager {
private:
  std::unique_ptr<IScene> _currentScene;
  TextureManager _textureManager;

public:
  void changeScene(std::unique_ptr<IScene> newScene) {
    if (_currentScene) {
      _currentScene->onExit();
    }
    _currentScene = std::move(newScene);
    if (_currentScene) {
        _currentScene->onEnter();
    }
  }

  void update(const std::vector<Zappy::Event> &events, Zappy::NetworkManager &networkManager, float deltaTime) {
    if (!_currentScene)
      return;

    auto netEvents = networkManager.consumeEvents();

    SceneState request = _currentScene->update(events, networkManager.getGameState(), netEvents, deltaTime);

    switch (request) {
    case SceneState::TITLE:
      changeScene(std::make_unique<MainTitle>(_textureManager));
      break;
    case SceneState::MENU:
      changeScene(std::make_unique<MenuScene>(_textureManager));
      break;

    case SceneState::NONE:
    default:
      break;
    }
  }

  void draw(Shader &shader) {
    if (_currentScene)
      _currentScene->draw(shader);
  }

  TextureManager &getTextureManager() { return _textureManager; }
};
} // namespace Zappy