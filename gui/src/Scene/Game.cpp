#include "Game.hpp"

void Zappy::GameScene::onEnter() {
  _renderer = std::make_unique<Renderer>(_windowSize.width, _windowSize.height);
  _texManager.get("gui/assets/island.png");
  _texManager.get("gui/assets/cute.png");
  _texManager.get("gui/assets/egg.png");

  for (int i = 0; i < 7; i++)
    _texManager.get("gui/assets/resource_" + std::to_string(i) + ".png");

  _textShader = std::make_unique<Shader>("gui/src/Core/Shader/text.vert",
                                         "gui/src/Core/Shader/text.frag");
  Font &feedFont =
      _fontManager.get("gui/assets/fonts/mainTitle.otf", 24.0f, 512);

  for (int i = 0; i < 5; i++) {
    auto t = std::make_unique<Text>(feedFont, "", 20.0f, 130.0f + i * 30.0f);
    t->color = Zappy::Math::vec3(0.3f, 1.0f, 0.3f);
    _broadcastTexts.push_back(std::move(t));
  }

  Font &goFont =
      _fontManager.get("gui/assets/fonts/mainTitle.otf", 64.0f, 1024);
  _gameOverText =
      std::make_unique<Text>(goFont, "", 0.0f, _windowSize.height / 2.0f);
  _gameOverText->color = Zappy::Math::vec3(1.0f, 0.8f, 0.0f);

  _texManager.get("gui/assets/incantation.png");
  _incantations.clear();

  _hud = std::make_unique<Hud>(_fontManager);
  _hud->onEnter();
}

void Zappy::GameScene::updateTileResources3D(int x, int z,
                                             const Zappy::Tile &tileData,
                                             float offX, float offZ) {
  int mapWidth = static_cast<int>(offX * 2.0f);
  int tileIndex = (z * mapWidth) + x;

  float baseX = (x - offX) * 2.0f;
  float baseZ = (z - offZ) * 2.0f;

  for (int i = 0; i < 7; i++) {
    int count = tileData.resources[i];

    if (count > 0 && !_tileVisuals[tileIndex].resourceSprites[i]) {
      Texture &resTex =
          _texManager.get("gui/assets/resource_" + std::to_string(i) + ".png");
      auto spr = std::make_unique<Sprite>(resTex);

      spr->position = Zappy::Math::vec3(baseX + RESOURCE_OFFSETS[i][0], 0.1f,
                                        baseZ + RESOURCE_OFFSETS[i][1]);
      spr->scale = Zappy::Math::vec3(0.4f, 0.4f, 0.4f);
      spr->rotation = Zappy::Math::vec3(90.0f, 0.0f, 0.0f);
      spr->isBillboard = true;

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
  static float globalCrystalTime = 0.0f;
  globalCrystalTime += deltaTime;

  if (_hud)
    _hud->update(gameState);

  if (!_isMapBuilt && gameState.map.isInitialized) {
    buildMap(gameState);
  }

  if (_isMapBuilt) {
    float offsetX = gameState.map.width / 2.0f;
    float offsetZ = gameState.map.height / 2.0f;
    bool isSpacePressed = false;
    bool isPPressed = false;

    for (const auto &event : events) {
      if (event.type == Zappy::EventType::KeyPressed &&
          event.keyCode == Zappy::Key::Space) {
        isSpacePressed = true;
      }
      if (event.type == Zappy::EventType::KeyPressed &&
          event.keyCode == Zappy::Key::P) {
        isPPressed = true;
      }
      if (event.type == Zappy::EventType::MouseWheelMove && _playerInventory) {
        if (!gameState.players.empty()) {
          std::vector<int> playersIds;
          for (const auto &[id, _] : gameState.players) {
            playersIds.push_back(id);
          }
          if (event.wheelDelta > 0)
            _currentPlayerIndex++;
          else if (event.wheelDelta < 0)
            _currentPlayerIndex--;
          if (_currentPlayerIndex >= (int)playersIds.size())
            _currentPlayerIndex = 0;
          else if (_currentPlayerIndex < 0)
            _currentPlayerIndex = playersIds.size() - 1;
          _playerInventory->setTargetPlayer(playersIds[_currentPlayerIndex]);
        }
      }
      if (event.type == Zappy::EventType::MousePressed && event.button == 1 &&
          !_quickMenu && !_playerInventory) {
        float x_ndc = (2.0f * event.mouseX) / _windowSize.width - 1.0f;
        float y_ndc = 1.0f - (2.0f * event.mouseY) / _windowSize.height;
        Zappy::Math::mat4 proj = Zappy::Math::perspective(
            Zappy::Math::radians(45.0f),
            (float)_windowSize.width / (float)_windowSize.height, 0.1f,
            1000.0f);
        float eyeX = x_ndc / proj.m[0];
        float eyeY = y_ndc / proj.m[5];
        float eyeZ = -1.0f;
        Zappy::Math::mat4 view = _camera.getViewMatrix();
        Zappy::Math::vec3 rayDir(
            eyeX * view.m[0] + eyeY * view.m[1] + eyeZ * view.m[2],
            eyeX * view.m[4] + eyeY * view.m[5] + eyeZ * view.m[6],
            eyeX * view.m[8] + eyeY * view.m[9] + eyeZ * view.m[10]);
        rayDir = Zappy::Math::normalize(rayDir);
        Zappy::Math::vec3 camPos(
            -(view.m[12] * view.m[0] + view.m[13] * view.m[1] +
              view.m[14] * view.m[2]),
            -(view.m[12] * view.m[4] + view.m[13] * view.m[5] +
              view.m[14] * view.m[6]),
            -(view.m[12] * view.m[8] + view.m[13] * view.m[9] +
              view.m[14] * view.m[10]));
        if (rayDir.y < 0.0f) {
          float t = -camPos.y / rayDir.y;
          float hitX = camPos.x + rayDir.x * t;
          float hitZ = camPos.z + rayDir.z * t;
          int mapX = std::round((hitX / 2.0f) + offsetX);
          int mapZ = std::round((hitZ / 2.0f) + offsetZ);
          if (mapX >= 0 && mapX < gameState.map.width && mapZ >= 0 &&
              mapZ < gameState.map.height) {
            int index = mapZ * gameState.map.width + mapX;
            _currentTileIndex = index;
            if (!_tileInventory) {
              _tileInventory = std::make_unique<tileInventory>(
                  _texManager, _networkManager, _fontManager);
              _tileInventory->onEnter();
            }
            _tileInventory->setTargetTile(gameState.grid[index]);
          } else {
            if (_tileInventory) {
              _tileInventory->onExit();
              _tileInventory.reset();
            }
          }
        }
      }
    }
    if (isPPressed && !_wasPPressed) {
      if (_playerInventory) {
        _playerInventory->onExit();
        _playerInventory.reset();
      } else {
        _playerInventory = std::make_unique<playerInventory>(
            _texManager, _networkManager, _fontManager);
        _playerInventory->onEnter();
        if (!gameState.players.empty()) {
          _currentPlayerIndex = 0;
          _playerInventory->setTargetPlayer(gameState.players.begin()->first);
        }
      }
    }
    if (isSpacePressed && !_wasSpacePressed) {
      if (_quickMenu) {
        _quickMenu->onExit();
        _quickMenu.reset();
      } else {
        _quickMenu = std::make_unique<quickMenu>(_texManager, _networkManager,
                                                 _windowSize);
        _quickMenu->onEnter();
      }
    }
    _wasPPressed = isPPressed;
    _wasSpacePressed = isSpacePressed;
    if (_quickMenu) {
      SceneState quickMenuState =
          _quickMenu->update(events, gameState, netEvents, deltaTime);
      if (quickMenuState != SceneState::NONE)
        return quickMenuState;
    }
    if (_tileInventory) {
      _tileInventory->update(events, gameState, netEvents, deltaTime);
    }
    if (_playerInventory) {
      _playerInventory->update(events, gameState, netEvents, deltaTime);
    }
    if (!_tileInventory && !_quickMenu && !_playerInventory)
      _camera.update(events);

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
          Texture &crystalTex = _texManager.get("gui/assets/crystal.png");
          auto crystalSprite = std::make_unique<Sprite>(crystalTex);
          crystalSprite->scale = Zappy::Math::vec3(0.3f, 0.3f, 0.3f);
          crystalSprite->isBillboard = true;
          crystalSprite->colorTint =
              getTeamColor(gameState.players.at(id).team);

          _playerCrystalMap[id] = crystalSprite.get();
          _crystals.push_back(std::move(crystalSprite));
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
      case Zappy::NetworkEventType::EGG_LAID: {
        if (netEvent.arguments.size() >= 5) {
          int eggId = cleanId(netEvent.arguments[1]);
          int x = std::stoi(netEvent.arguments[3]);
          int y = std::stoi(netEvent.arguments[4]);
          spawnEgg3D(eggId, x, y, offsetX, offsetZ);
        }
        break;
      }

      case Zappy::NetworkEventType::EGG_HATCHED:
      case Zappy::NetworkEventType::EGG_DIED: {
        if (netEvent.arguments.size() >= 2) {
          int eggId = cleanId(netEvent.arguments[1]);
          removeEgg3D(eggId);
        }
        break;
      }

      case Zappy::NetworkEventType::BROADCAST: {
        if (netEvent.arguments.size() >= 3) {
          std::string sender = netEvent.arguments[1];
          std::string fullMsg = "";
          for (size_t i = 2; i < netEvent.arguments.size(); ++i) {
            fullMsg += netEvent.arguments[i] +
                       (i == netEvent.arguments.size() - 1 ? "" : " ");
          }
          addBroadcastLog(sender, fullMsg);
          int senderId = cleanId(sender);

          if (gameState.players.contains(senderId)) {
             _playerAnims[senderId] = {"jump", 0.0f};
          }
        }
        break;
      }
      case Zappy::NetworkEventType::RESOURCE_COLLECTED:
      case Zappy::NetworkEventType::RESOURCE_DROPPED: {
        if (netEvent.arguments.size() >= 2) {
          _playerAnims[cleanId(netEvent.arguments[1])] = {"jump", 0.0f};
        }
        break;
      }
      case Zappy::NetworkEventType::PLAYER_EXPULSED: {
        if (netEvent.arguments.size() >= 2) {
          _playerAnims[cleanId(netEvent.arguments[1])] = {"shake", 0.0f};
        }
        break;
      }
      case Zappy::NetworkEventType::EGG_LAYING: {
        if (netEvent.arguments.size() >= 2) {
          _playerAnims[cleanId(netEvent.arguments[1])] = {"squeeze", 0.0f};
        }
        break;
      }
      case Zappy::NetworkEventType::GAME_OVER: {
        if (netEvent.arguments.size() >= 2) {
          _isGameOver = true;
          _gameOverText->setString("VICTORY FOR TEAM " + netEvent.arguments[1]);
          _gameOverText->setPosition((_windowSize.width / 2.0f) -
                                         (_gameOverText->getWidth() / 2.0f),
                                     _windowSize.height / 2.0f);
        }
        break;
      }
      case Zappy::NetworkEventType::SERVER_MESSAGE: {
        if (netEvent.arguments.size() >= 2) {
          addBroadcastLog("SERVER", netEvent.arguments[1]);
        }
        break;
      }
      case Zappy::NetworkEventType::INCANTATION_START: {
        if (netEvent.arguments.size() >= 4) {
          int x = std::stoi(netEvent.arguments[1]);
          int y = std::stoi(netEvent.arguments[2]);
          Texture &magicTex = _texManager.get("gui/assets/incantation.png");
          auto magicSprite = std::make_unique<Sprite>(magicTex);
          magicSprite->isBillboard = true;
          _incantations.push_back({x, y, 0.0f, std::move(magicSprite)});
          addBroadcastLog("SERVER", "Incantation en (" + std::to_string(x) +
                                        ", " + std::to_string(y) + ")");
        }
        break;
      }
      case Zappy::NetworkEventType::INCANTATION_END: {
        if (netEvent.arguments.size() >= 4) {
          int x = std::stoi(netEvent.arguments[1]);
          int y = std::stoi(netEvent.arguments[2]);
          int result = std::stoi(netEvent.arguments[3]);

          _incantations.erase(
              std::remove_if(_incantations.begin(), _incantations.end(),
                             [x, y](const ActiveIncantation &inc) {
                               return inc.x == x && inc.y == y;
                             }),
              _incantations.end());

          if (result == 0) {
            for (const auto &[id, p] : gameState.players) {
              if (p.x == x && p.y == y) {
                _playerAnims[id] = {"shake", 0.0f};
              }
            }
          }
        }
        break;
      }
      case Zappy::NetworkEventType::PLAYER_LEVEL_UP: {
        if (netEvent.arguments.size() >= 3) {
          int id = cleanId(netEvent.arguments[1]);
          _playerAnims[id] = {"levelup", 0.0f};
        }
        break;
      }

      default:
        break;
      }
    }

    for (const auto &[id, p] : gameState.players) {
      if (_playerMap.contains(id)) {
        float baseHeight = 0.0f;
        float animOffsetX = 0.0f;
        float scaleX = 1.0f, scaleY = 1.0f;

        if (_playerAnims.contains(id)) {
          _playerAnims[id].timer += deltaTime;
          float t = _playerAnims[id].timer;

          if (_playerAnims[id].type == "jump") {
            if (t < 0.3f) {
              baseHeight += std::sin(t / 0.3f * 3.14159f) * 0.5f;
            } else {
              _playerAnims.erase(id);
            }
          } else if (_playerAnims[id].type == "shake") {
            if (t < 0.3f) {
              animOffsetX = std::sin(t * 50.0f) * 0.2f;
            } else {
              _playerAnims.erase(id);
            }
          } else if (_playerAnims[id].type == "squeeze") {
            if (t < 1.0f) {
              float squeeze = std::sin(t * 10.0f) * 0.2f;
              scaleX = 1.0f + squeeze;
              scaleY = 1.0f - squeeze;
            } else {
              _playerAnims.erase(id);
            }
          } else if (_playerAnims[id].type == "levelup") {
            if (t < 1.0f) {
              _playerMap[id]->rotation.y = t * 360.0f * 2.0f;
              scaleX = 1.0f + std::sin(t * 3.14159f) * 0.5f;
              scaleY = scaleX;
            } else {
              _playerMap[id]->rotation.y = 0.0f;
              _playerAnims.erase(id);
            }
          }
        }

        _playerMap[id]->scale = Zappy::Math::vec3(scaleX, scaleY, 1.0f);

        Zappy::Math::vec3 targetPos((p.x - offsetX) * 2.0f + animOffsetX,
                                    baseHeight, (p.y - offsetZ) * 2.0f - 1.0f);

        float dx = targetPos.x - _playerMap[id]->position.x;
        float dz = targetPos.z - _playerMap[id]->position.z;

        if (std::abs(dx) > 3.0f || std::abs(dz) > 3.0f) {
          _playerMap[id]->position = targetPos;
        } else {
          float lerpSpeed = 10.0f * deltaTime;
          if (lerpSpeed > 1.0f)
            lerpSpeed = 1.0f;
          if (std::abs(dx) > 0.05f || std::abs(dz) > 0.05f) {
            _playerMap[id]->position.y = baseHeight + std::abs(std::sin(globalCrystalTime * 15.0f)) * 0.3f;
          } else {
            _playerMap[id]->position.y = baseHeight;
          }

          _playerMap[id]->position = Zappy::Math::transi(
              _playerMap[id]->position, targetPos, lerpSpeed);
        }
        if (_playerCrystalMap.contains(id)) {
          float hoverY = 1.0f + std::sin(globalCrystalTime * 3.0f + id) * 0.15f;
          _playerCrystalMap[id]->position = _playerMap[id]->position;
          _playerCrystalMap[id]->position.y += hoverY;
        }
      }
    }

    for (auto &inc : _incantations) {
      inc.timer += deltaTime;
      inc.sprite->position = Zappy::Math::vec3((inc.x - offsetX) * 2.0f, 0.5f,
                                               (inc.y - offsetZ) * 2.0f - 1.0f);
      float pulse = 1.5f + std::sin(inc.timer * 6.0f) * 0.3f;
      inc.sprite->scale = Zappy::Math::vec3(pulse, pulse, pulse);
    }
  }

  for (auto it = _broadcastLogs.begin(); it != _broadcastLogs.end();) {
    it->timer += deltaTime;
    if (it->timer >= 5.0f) {
      it = _broadcastLogs.erase(it);
    } else {
      it++;
    }
  }

  for (auto it = _dyingEntities.begin(); it != _dyingEntities.end();) {
    it->timer += deltaTime;

    float progress = std::min(it->timer / 1.0f, 1.0f);
    it->sprite->rotation.x = Zappy::Math::transi(0.0f, -1.5708f, progress);
    it->sprite->position.y = Zappy::Math::transi(0.0f, -0.5f, progress);

    if (it->timer >= 1.0f) {
      it = _dyingEntities.erase(it);
    } else {
      it++;
    }
  }

  return SceneState::NONE;
}

void Zappy::GameScene::draw(Shader &shader, WindowSize &windowSize) {
  _windowSize = windowSize;
  if (_renderer && _isMapBuilt && _floor) {
    std::vector<std::reference_wrapper<Sprite>> resourcesToDraw;

    for (const auto &[index, tile] : _tileVisuals) {
      for (int i = 0; i < 7; i++) {
        if (tile.resourceSprites[i]) {
          resourcesToDraw.push_back(*tile.resourceSprites[i]);
        }
      }
    }

    for (auto &eggSprite : _eggs) {
      resourcesToDraw.push_back(*eggSprite);
    }

    for (auto &dying : _dyingEntities) {
      resourcesToDraw.push_back(*dying.sprite);
    }

    for (auto &inc : _incantations) {
      resourcesToDraw.push_back(*inc.sprite);
    }

    for (auto &c : _crystals) {
      resourcesToDraw.push_back(*c);
    }

    _renderer->render(_camera, *_floor, _players, resourcesToDraw, windowSize, _dynamicFocusRange);
  }
  if (_hud)
    _hud->draw();
  if (_quickMenu) {
    glDisable(GL_DEPTH_TEST);
    _quickMenu->draw(shader, windowSize);
    glEnable(GL_DEPTH_TEST);
  }
  if (_tileInventory) {
    glDisable(GL_DEPTH_TEST);
    _tileInventory->draw(shader, windowSize);
    glEnable(GL_DEPTH_TEST);
  }
  if (_playerInventory) {
    glDisable(GL_DEPTH_TEST);
    _playerInventory->draw(shader, windowSize);
    glEnable(GL_DEPTH_TEST);
  }
  if (!_broadcastLogs.empty()) {
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (!_broadcastLogs.empty()) {
      glDisable(GL_DEPTH_TEST);
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

      Zappy::Math::mat4 orthoProj = Zappy::Math::ortho(
          0.0f, _windowSize.width, _windowSize.height, 0.0f, -1.0f, 1.0f);

      for (size_t i = 0;
           i < _broadcastLogs.size() && i < _broadcastTexts.size(); i++) {
        _broadcastTexts[i]->setString(_broadcastLogs[i].text);

        float t = _broadcastLogs[i].timer;
        float alpha = 1.0f;

        if (t < 0.5f) {
          alpha = t / 0.5f;
        } else if (t > 4.0f) {
          alpha = (5.0f - t) / 1.0f;
        }

        _broadcastTexts[i]->alpha = alpha;
        _broadcastTexts[i]->draw(*_textShader, orthoProj);
      }
    }

    if (_isGameOver && _gameOverText) {
      glDisable(GL_DEPTH_TEST);
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

      Zappy::Math::mat4 orthoProj = Zappy::Math::ortho(
          0.0f, _windowSize.width, _windowSize.height, 0.0f, -1.0f, 1.0f);

      static float goTimer = 0.0f;
      goTimer += 0.016f;
      _gameOverText->alpha = 0.5f + std::sin(goTimer * 5.0f) * 0.5f;

      _gameOverText->draw(*_textShader, orthoProj);

      glDisable(GL_BLEND);
      glEnable(GL_DEPTH_TEST);
    }
  }
}

void Zappy::GameScene::onExit() {
  _players.clear();
  _playerMap.clear();
  _eggs.clear();
  _eggMap.clear();
  _broadcastLogs.clear();
  _floor.reset();
  _renderer.reset();
  _textShader.reset();
  _broadcastTexts.clear();
  _playerAnims.clear();
  _isGameOver = false;
  _gameOverText.reset();
  if (_hud) {
    _hud->onExit();
    _hud.reset();
  }
  if (_quickMenu)
    _quickMenu->onExit();
  if (_tileInventory)
    _tileInventory->onExit();
}

void Zappy::GameScene::buildMap(const Zappy::GameState &gameState) {
  Texture &islandTex = _texManager.get("gui/assets/island.png");
  Texture &cuteTex = _texManager.get("gui/assets/cute.png");

  _floor = std::make_unique<InstancedGrid>(islandTex);
  _players.clear();
  _playerMap.clear();

  float offsetX = gameState.map.width / 2.0f;
  float offsetZ = gameState.map.height / 2.0f;

  float maxDimension = std::max(gameState.map.width, gameState.map.height);
  _dynamicFocusRange = maxDimension * 1.5f;

  if (_dynamicFocusRange < 15.0f)
        _dynamicFocusRange = 15.0f;

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
    auto it = std::find_if(_players.begin(), _players.end(),
                           [targetSprite](const std::unique_ptr<Sprite> &s) {
                             return s.get() == targetSprite;
                           });

    if (it != _players.end()) {
      (*it)->isBillboard = false;
      _dyingEntities.push_back({std::move(*it), 0.0f});
      _players.erase(it);
    }
    _playerMap.erase(id);
  }
  if (_playerCrystalMap.contains(id)) {
    Sprite *cTarget = _playerCrystalMap[id];
    auto cIt = std::find_if(_crystals.begin(), _crystals.end(),
                            [cTarget](const std::unique_ptr<Sprite> &s) {
                              return s.get() == cTarget;
                            });
    if (cIt != _crystals.end()) {
      (*cIt)->isBillboard = false;
      _dyingEntities.push_back({std::move(*cIt), 0.0f});
      _crystals.erase(cIt);
    }
    _playerCrystalMap.erase(id);
  }
}

int Zappy::GameScene::cleanId(const std::string &idStr) {
  if (idStr.empty())
    return 0;
  return (idStr[0] == '#') ? std::stoi(idStr.substr(1)) : std::stoi(idStr);
}

void Zappy::GameScene::spawnEgg3D(int eggId, int x, int y, float offX,
                                  float offZ) {
  if (_eggMap.contains(eggId))
    return;

  Texture &eggTex = _texManager.get("gui/assets/egg.png");
  auto eggSprite = std::make_unique<Sprite>(eggTex);

  eggSprite->position =
      Zappy::Math::vec3((x - offX) * 2.0f, 0.1f, (y - offZ) * 2.0f - 0.5f);
  eggSprite->scale = Zappy::Math::vec3(0.5f, 0.5f, 0.5f);
  eggSprite->isBillboard = true;

  _eggMap[eggId] = eggSprite.get();
  _eggs.push_back(std::move(eggSprite));

  LOG_INFO("GUI: Egg spawned with ID #" + std::to_string(eggId) + " at (" +
           std::to_string(x) + ", " + std::to_string(y) + ")");
}

void Zappy::GameScene::removeEgg3D(int eggId) {
  if (_eggMap.contains(eggId)) {
    Sprite *target = _eggMap[eggId];
    auto it = std::find_if(_eggs.begin(), _eggs.end(),
                           [target](const std::unique_ptr<Sprite> &s) {
                             return s.get() == target;
                           });

    if (it != _eggs.end()) {
      (*it)->isBillboard = false;
      _dyingEntities.push_back({std::move(*it), 0.0f});
      _eggs.erase(it);
    }
    _eggMap.erase(eggId);
  }
}

void Zappy::GameScene::addBroadcastLog(const std::string &sender,
                                       const std::string &message) {
  std::string entry = "id: " + sender + " broadcasted: " + message;

  LOG_INFO(entry);
  _broadcastLogs.insert(_broadcastLogs.begin(), {entry, 0.0f});

  if (_broadcastLogs.size() > 5) {
    _broadcastLogs.pop_back();
  }
}

Zappy::Math::vec3 Zappy::GameScene::getTeamColor(const std::string &teamName) {
  if (!_teamColors.contains(teamName)) {
    float r = (rand() % 155 + 100) / 255.0f;
    float g = (rand() % 155 + 100) / 255.0f;
    float b = (rand() % 155 + 100) / 255.0f;
    _teamColors[teamName] = Zappy::Math::vec3(r, g, b);
  }
  return _teamColors[teamName];
}