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
#include <map>
#include <algorithm>

namespace Zappy {
class MenuScene : public IScene {
private:
  TextureManager &_texManager;
  std::unique_ptr<Renderer> _renderer;

  Camera _camera;
  std::unique_ptr<InstancedGrid> _floor;
  std::vector<std::unique_ptr<Sprite>> _players;
  std::map<int, Sprite*> _playerMap;

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
    _camera.update(events);

    if (!_isMapBuilt && gameState.map.isInitialized) {
      buildMap(gameState);
    }

    if (_isMapBuilt) {
        float offsetX = gameState.map.width / 2.0f;
        float offsetZ = gameState.map.height / 2.0f;

        for (const auto& netEvent : netEvents) {
            if (netEvent.type == NetworkEventType::PLAYER_CONNECTED) {
                int id = (netEvent.arguments[1][0] == '#') ? std::stoi(netEvent.arguments[1].substr(1)) : std::stoi(netEvent.arguments[1]);
                if (gameState.players.contains(id) && !_playerMap.contains(id)) {
                    Texture &cuteTex = _texManager.get("gui/assets/cute.png");
                    spawnPlayer3D(id, gameState.players.at(id), cuteTex, offsetX, offsetZ);
                }
            } 
            else if (netEvent.type == NetworkEventType::PLAYER_DISCONNECTED) {
                int id = (netEvent.arguments[1][0] == '#') ? std::stoi(netEvent.arguments[1].substr(1)) : std::stoi(netEvent.arguments[1]);
                removePlayer3D(id);
            }
        }

        for (const auto& [id, p] : gameState.players) {
            if (_playerMap.contains(id)) {
                _playerMap[id]->position = Zappy::Math::vec3((p.x - offsetX) * 2.0f, 0.0f, (p.y - offsetZ) * 2.0f - 1.0f);
            }
        }
    }

    return SceneState::NONE;
  }

  void draw(Shader &) override {
    if (_renderer && _isMapBuilt && _floor) {
      _renderer->render(_camera, *_floor, _players);
    }
  }

  void onExit() override {
    _players.clear();
    _playerMap.clear();
    _floor.reset();
    _renderer.reset();
  }

private:
  void buildMap(const Zappy::GameState &gameState) {
    Texture &islandTex = _texManager.get("gui/assets/island.png");
    Texture &cuteTex = _texManager.get("gui/assets/cute.png");

    _floor = std::make_unique<InstancedGrid>(islandTex);
    _players.clear();
    _playerMap.clear();

    float offsetX = gameState.map.width / 2.0f;
    float offsetZ = gameState.map.height / 2.0f;

    for (int x = 0; x < gameState.map.width; x++) {
      for (int z = 0; z < gameState.map.height; z++) {
        _floor->addTile(Zappy::Math::vec3((x - offsetX) * 2.0f, 0.0f, (z - offsetZ) * 2.0f), 2.0f);
      }
    }
    _floor->build();

    for (const auto &[id, player] : gameState.players) {
        spawnPlayer3D(id, player, cuteTex, offsetX, offsetZ);
    }

    _isMapBuilt = true;
  }

  void spawnPlayer3D(int id, const Zappy::Player& p, Texture& tex, float offX, float offZ) {
    auto playerSprite = std::make_unique<Sprite>(tex);
    
    playerSprite->position = Zappy::Math::vec3((p.x - offX) * 2.0f, 0.0f, (p.y - offZ) * 2.0f - 1.0f);
    playerSprite->scale = Zappy::Math::vec3(1.0f, 1.0f, 1.0f);
    playerSprite->isBillboard = true;

    _playerMap[id] = playerSprite.get();
    _players.push_back(std::move(playerSprite));
  }

  void removePlayer3D(int id) {
      if (_playerMap.contains(id)) {
          Sprite* targetSprite = _playerMap[id];

          auto it = std::remove_if(_players.begin(), _players.end(), 
            [targetSprite](const std::unique_ptr<Sprite>& s) { return s.get() == targetSprite; });
            
          if (it != _players.end()) {
              _players.erase(it, _players.end());
          }
          _playerMap.erase(id);
      }
  }
};
} // namespace Zappy