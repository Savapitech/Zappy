#pragma once
#include "IScene/IScene.hpp"
#include "Logger.hpp"
#include "Sprite/InstancedGrid.hpp"
#include "Sprite/Sprite.hpp"
#include "Texture/TextureManager.hpp"
#include "Utils/math.hpp"

#include "Render/Camera.hpp"
#include "Render/Render.hpp"

#include <memory>
#include <vector>

namespace Zappy {
class MenuScene : public IScene {
private:
  TextureManager &_texManager;
  std::unique_ptr<Renderer> _renderer;

  Camera _camera;
  std::unique_ptr<InstancedGrid> _floor;
  std::vector<std::unique_ptr<Sprite>> _players;

public:
  MenuScene(TextureManager &tm) : _texManager(tm) {}

  void onEnter() override {
    _renderer = std::make_unique<Renderer>(WIDTH, HEIGHT);

    Texture &islandTex =
        _texManager.get("gui/assets/island.png");
    Texture &cuteTex =
        _texManager.get("gui/assets/cute.png");

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
  }

  SceneState update(const std::vector<Zappy::Event> &events, float deltaTime) override {
    _camera.update(events);

    return SceneState::NONE;
  }

  void draw(Shader &) override {
    if (_renderer && _floor) {
      _renderer->render(_camera, *_floor, _players);
    }
  }

  void onExit() override {
    _players.clear();
    _floor.reset();
    _renderer.reset();
  }
};
} // namespace Zappy 