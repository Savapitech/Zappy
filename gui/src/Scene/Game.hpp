#pragma once
#include "IScene/IScene.hpp"
#include "Logger.hpp"
#include "Sprite/InstancedGrid.hpp"
#include "Sprite/Sprite.hpp"
#include "Scene/QuickMenu.hpp"
#include "Scene/TileInventory.hpp"
#include "Scene/PlayerInventory.hpp"
#include "Texture/TextureManager.hpp"
#include "Utils/math.hpp"
#include "Network/NetworkManager.hpp"
#include "Render/Camera.hpp"
#include "Render/Render.hpp"
#include "Text/Text.hpp"
#include "Font/FontManager.hpp"

#include <algorithm>
#include <map>
#include <memory>
#include <vector>
#include <cstdlib>

namespace Zappy {

struct DyingEntity {
    std::unique_ptr<Sprite> sprite;
    float timer;
};

struct BroadcastMsg {
  std::string text;
  float timer;
};

class GameScene : public IScene {
private:
  TextureManager &_texManager;
  std::unique_ptr<Renderer> _renderer;
  int _currentTileIndex = 0;
  int _currentPlayerIndex = 0;
  Camera _camera;
  std::unique_ptr<InstancedGrid> _floor;
  std::vector<std::unique_ptr<Sprite>> _players;
  std::map<int, Sprite *> _playerMap;
  std::unique_ptr<IScene> _quickMenu = nullptr;
  bool _wasSpacePressed = false;
  
  std::unique_ptr<tileInventory> _tileInventory;
  std::unique_ptr<playerInventory> _playerInventory;
  bool _wasPPressed = false;
  bool _isMapBuilt;
  Zappy::NetworkManager &_networkManager;

  std::vector<std::unique_ptr<Sprite>> _eggs;
  std::map<int, Sprite *> _eggMap;

  FontManager _fontManager;
  std::unique_ptr<Shader> _textShader;

  std::vector<std::unique_ptr<Text>> _broadcastTexts;
  std::vector<BroadcastMsg> _broadcastLogs;

  std::vector<DyingEntity> _dyingEntities;

  struct PlayerAnim {
      std::string type;
      float timer;
  };
  std::map<int, PlayerAnim> _playerAnims;

  bool _isGameOver = false;
  std::unique_ptr<Text> _gameOverText;

  struct ActiveIncantation {
      int x;
      int y;
      float timer;
      std::unique_ptr<Sprite> sprite;
  };
  std::vector<ActiveIncantation> _incantations;

  std::vector<std::unique_ptr<Sprite>> _crystals;
  std::map<int, Sprite *> _playerCrystalMap;
  std::map<std::string, Zappy::Math::vec3> _teamColors;
  Zappy::Math::vec3 getTeamColor(const std::string& teamName);

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

  void spawnEgg3D(int eggId, int x, int y, float offX, float offZ);
  void removeEgg3D(int eggId);
  void addBroadcastLog(const std::string &sender, const std::string &message);
  int cleanId(const std::string &idStr);

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
