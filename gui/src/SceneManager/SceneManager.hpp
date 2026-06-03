#pragma once
#include <memory>
#include "IScene/IScene.hpp"
#include "Scene/Menu.hpp"
#include "Texture/TextureManager.hpp"

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

        void update() {
            if (!_currentScene)
                return;

        SceneState request = _currentScene->update();

        switch (request) {
            case SceneState::MENU:
                changeScene(std::make_unique<MenuScene>(_textureManager));
                break;

            case SceneState::NONE:
            default:
                break;
        }
    }

        void draw(Shader& shader) {
            if (_currentScene)
                _currentScene->draw(shader);
        }

        TextureManager& getTextureManager() { return _textureManager; }
    };
}
