#pragma once
#include "IScene/IScene.hpp"
#include "Scene/Menu.hpp"
#include "Texture/TextureManager.hpp"
#include <memory>

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
    _currentScene->onEnter();
  }

  void update(const std::vector<Zappy::Event> &events) {
    if (!_currentScene)
      return;

    SceneState request = _currentScene->update(events);

    switch (request) {
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
