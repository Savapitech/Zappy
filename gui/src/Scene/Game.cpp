#include "Game.hpp"

void Zappy::GameScene::onEnter() {
  _renderer = std::make_unique<Renderer>(WIDTH, HEIGHT);
  _texManager.get("gui/assets/island.png");
  _texManager.get("gui/assets/cute.png");

  for (int i = 0; i < 7; ++i)
    _texManager.get("gui/assets/resource_" + std::to_string(i) + ".png");
}

void Zappy::GameScene::updateTileResources3D(int x, int z,
                                             const Zappy::Tile &tileData,
                                             float offX, float offZ) {
  int mapWidth = static_cast<int>(offX * 2.0f);
  int tileIndex = (z * mapWidth) + x;

  float baseX = (x - offX) * 2.0f;
  float baseZ = (z - offZ) * 2.0f;

  for (int i = 0; i < 7; ++i) {
    int count = tileData.resources[i];

    if (count > 0 && !_tileVisuals[tileIndex].resourceSprites[i]) {
      Texture &resTex =
          _texManager.get("gui/assets/resource_" + std::to_string(i) + ".png");
      auto spr = std::make_unique<Sprite>(resTex);

      spr->position = Zappy::Math::vec3(baseX + RESOURCE_OFFSETS[i][0], 1.0f,
                                        baseZ + RESOURCE_OFFSETS[i][1]);
      spr->scale = Zappy::Math::vec3(0.4f, 0.4f, 0.4f);
      spr->rotation = Zappy::Math::vec3(90.0f, 0.0f, 0.0f);
      spr->isBillboard = false;

      _tileVisuals[tileIndex].resourceSprites[i] = std::move(spr);
    }

    else if (count == 0 && _tileVisuals[tileIndex].resourceSprites[i]) {
      _tileVisuals[tileIndex].resourceSprites[i].reset();
    }
  }
}

Zappy::SceneState Zappy::GameScene::update(
    const std::vector<Zappy::Event> &events, const Zappy::GameState &gameState,
    const std::vector<Zappy::NetworkEvent> &netEvents, float deltaTime) {
  _camera.update(events);

  if (!_isMapBuilt && gameState.map.isInitialized) {
    buildMap(gameState);
  }

  if (_isMapBuilt) {
    float offsetX = gameState.map.width / 2.0f;
    float offsetZ = gameState.map.height / 2.0f;

    for (const auto &netEvent : netEvents) {

      switch (netEvent.type) {

      case Zappy::NetworkEventType::PLAYER_CONNECTED: {
        int id = (netEvent.arguments[1][0] == '#')
                     ? std::stoi(netEvent.arguments[1].substr(1))
                     : std::stoi(netEvent.arguments[1]);
        if (gameState.players.contains(id) && !_playerMap.contains(id)) {
          Texture &cuteTex = _texManager.get("gui/assets/cute.png");
          spawnPlayer3D(id, gameState.players.at(id), cuteTex, offsetX,
                        offsetZ);
        }
        break;
      }

      case Zappy::NetworkEventType::PLAYER_DISCONNECTED: {
        int id = (netEvent.arguments[1][0] == '#')
                     ? std::stoi(netEvent.arguments[1].substr(1))
                     : std::stoi(netEvent.arguments[1]);
        removePlayer3D(id);
        break;
      }

      case Zappy::NetworkEventType::TILE_UPDATED: {
        int x = std::stoi(netEvent.arguments[1]);
        int z = std::stoi(netEvent.arguments[2]);

        int index = z * gameState.map.width + x;
        updateTileResources3D(x, z, gameState.grid[index], offsetX, offsetZ);
        break;
      }

      default:
        break;
      }
    }

    for (const auto &[id, p] : gameState.players) {
      if (_playerMap.contains(id)) {
        _playerMap[id]->position = Zappy::Math::vec3(
            (p.x - offsetX) * 2.0f, 0.0f, (p.y - offsetZ) * 2.0f - 1.0f);
      }
    }
  }

  return SceneState::NONE;
}

void Zappy::GameScene::draw(Shader &) {
  if (_renderer && _isMapBuilt && _floor) {
    std::vector<std::reference_wrapper<Sprite>> resourcesToDraw;

    for (const auto &[index, tile] : _tileVisuals) {
      for (int i = 0; i < 7; ++i) {
        if (tile.resourceSprites[i]) {
          resourcesToDraw.push_back(*tile.resourceSprites[i]);
        }
      }
    }
    _renderer->render(_camera, *_floor, _players, resourcesToDraw);
  }
}

void Zappy::GameScene::onExit() {
  _players.clear();
  _playerMap.clear();
  _floor.reset();
  _renderer.reset();
}

void Zappy::GameScene::buildMap(const Zappy::GameState &gameState) {
  Texture &islandTex = _texManager.get("gui/assets/island.png");
  Texture &cuteTex = _texManager.get("gui/assets/cute.png");

  _floor = std::make_unique<InstancedGrid>(islandTex);
  _players.clear();
  _playerMap.clear();

  float offsetX = gameState.map.width / 2.0f;
  float offsetZ = gameState.map.height / 2.0f;

  for (int x = 0; x < gameState.map.width; x++) {
    for (int z = 0; z < gameState.map.height; z++) {
      _floor->addTile(
          Zappy::Math::vec3((x - offsetX) * 2.0f, 0.0f, (z - offsetZ) * 2.0f),
          2.0f);

      int index = z * gameState.map.width + x;
      updateTileResources3D(x, z, gameState.grid[index], offsetX, offsetZ);
    }
  }
  _floor->build();

  for (const auto &[id, player] : gameState.players) {
    spawnPlayer3D(id, player, cuteTex, offsetX, offsetZ);
  }

  _isMapBuilt = true;
}

void Zappy::GameScene::spawnPlayer3D(int id, const Zappy::Player &p,
                                     Texture &tex, float offX, float offZ) {
  auto playerSprite = std::make_unique<Sprite>(tex);

  playerSprite->position =
      Zappy::Math::vec3((p.x - offX) * 2.0f, 0.0f, (p.y - offZ) * 2.0f - 1.0f);
  playerSprite->scale = Zappy::Math::vec3(1.0f, 1.0f, 1.0f);
  playerSprite->isBillboard = true;

  _playerMap[id] = playerSprite.get();
  _players.push_back(std::move(playerSprite));
}

void Zappy::GameScene::removePlayer3D(int id) {
  if (_playerMap.contains(id)) {
    Sprite *targetSprite = _playerMap[id];

    auto it = std::remove_if(_players.begin(), _players.end(),
                             [targetSprite](const std::unique_ptr<Sprite> &s) {
                               return s.get() == targetSprite;
                             });

    if (it != _players.end()) {
      _players.erase(it, _players.end());
    }
    _playerMap.erase(id);
  }
}