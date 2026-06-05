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

  bool _isMapBuilt;

public:
  MenuScene(TextureManager &tm) : _texManager(tm), _isMapBuilt(false) {}

  void onEnter() override {
    _renderer = std::make_unique<Renderer>(WIDTH, HEIGHT);

    _texManager.get("gui/assets/island.png");
    _texManager.get("gui/assets/cute.png");
  }

  SceneState update(const std::vector<Zappy::Event> &events,
                    const Zappy::GameState &gameState,
                    const std::vector<Zappy::NetworkEvent> &netEvents) override 
  {
    
    if (!_isMapBuilt && gameState.map.isInitialized)
      buildMap(gameState);

    
    _camera.update(events);

    return SceneState::NONE;
  }

  void draw(Shader &) override {
    if (_renderer && _isMapBuilt && _floor) {
      _renderer->render(_camera, *_floor, _players);
    }
  }

  void onExit() override {
    _players.clear();
    _floor.reset();
    _renderer.reset();
  }

private:
  void buildMap(const Zappy::GameState &gameState) {
    Texture &islandTex = _texManager.get("gui/assets/island.png");
    Texture &cuteTex = _texManager.get("gui/assets/cute.png");

    _floor = std::make_unique<InstancedGrid>(islandTex);
    _players.clear();

    float offsetX = gameState.map.width / 2.0f;
    float offsetZ = gameState.map.height / 2.0f;

    for (int x = 0; x < gameState.map.width; x++) {
      for (int z = 0; z < gameState.map.height; z++) {
        _floor->addTile(Zappy::Math::vec3((x - offsetX) * 2.0f, 0.0f, (z - offsetZ) * 2.0f), 2.0f);
      }
    }
    _floor->build();

    for (const auto &[id, player] : gameState.players) {
        spawnPlayer3D(player, cuteTex, offsetX, offsetZ);
    }

    _isMapBuilt = true;

    LOG_INFO("Map built dynamically: " + std::to_string(gameState.map.width) + "x" + std::to_string(gameState.map.height));
  }

  void spawnPlayer3D(const Zappy::Player& p, Texture& tex, float offX, float offZ) {
    auto playerSprite = std::make_unique<Sprite>(tex);
    playerSprite->position = Zappy::Math::vec3((p.x - offX) * 2.0f, 0.0f, (p.y - offZ) * 2.0f - 1.0f);
    playerSprite->scale = Zappy::Math::vec3(1.0f, 1.0f, 1.0f);
    playerSprite->isBillboard = true;
    
    _players.push_back(std::move(playerSprite));
  }
};
} // namespace Zappy