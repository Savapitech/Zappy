#pragma once
#include "IScene/IScene.hpp"
#include "Logger.hpp"
#include "Sprite/InstancedGrid.hpp"
#include "Sprite/Sprite.hpp"
#include "Scene/QuickMenu.hpp"
#include "Scene/TileInventory.hpp"
#include "Texture/TextureManager.hpp"
#include "Utils/math.hpp"
#include "Network/NetworkManager.hpp"
#include "Render/Camera.hpp"
#include "Render/Render.hpp"

#include <algorithm>
#include <map>
#include <memory>
#include <vector>

namespace Zappy {
class GameScene : public IScene {
private:
  TextureManager &_texManager;
  std::unique_ptr<Renderer> _renderer;
  
  Camera _camera;
  std::unique_ptr<InstancedGrid> _floor;
  std::vector<std::unique_ptr<Sprite>> _players;
  std::map<int, Sprite *> _playerMap;
  std::unique_ptr<IScene> _quickMenu = nullptr;
  bool _wasSpacePressed = false;
  
  std::unique_ptr<IScene> _tileInventory;
  bool _wasTPressed = false;
  bool _isMapBuilt;
  Zappy::NetworkManager &_networkManager;

public:
  GameScene(TextureManager &tm, Zappy::NetworkManager &nm) : _texManager(tm), _isMapBuilt(false), _networkManager(nm) {}

  void onEnter() override;

  SceneState update(const std::vector<Zappy::Event> &events,
                    const Zappy::GameState &gameState,
                    const std::vector<Zappy::NetworkEvent> &netEvents,
                    float deltaTime) override;

  void draw(Shader &) override;

  void onExit() override;

private:
  void buildMap(const Zappy::GameState &gameState);
  void updateTileResources3D(int x, int z, const Zappy::Tile &tileData,
                             float offX, float offZ);
  void spawnPlayer3D(int id, const Zappy::Player &p, Texture &tex, float offX,
                     float offZ);
  void removePlayer3D(int id);

  const float RESOURCE_OFFSETS[7][2] = {
      {0.0f, 0.0f},   // Food
      {-0.3f, -0.3f}, // Linemate
      {0.3f, -0.3f},  // Deraumere
      {-0.3f, 0.3f},  // Sibur
      {0.3f, 0.3f},   // Mendiane
      {-0.3f, 0.0f},  // Phiras
      {0.3f, 0.0f}    // Thystame
  };

  struct TileVisual {
    std::unique_ptr<Sprite> resourceSprites[7];
  };

  std::unordered_map<int, TileVisual> _tileVisuals;
};
} // namespace Zappy
