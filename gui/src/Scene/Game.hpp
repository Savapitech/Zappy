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
class GameScene : public IScene {
private:
  TextureManager &_texManager;
  std::unique_ptr<Renderer> _renderer;

  Camera _camera;
  std::unique_ptr<InstancedGrid> _floor;
  std::vector<std::unique_ptr<Sprite>> _players;
  std::map<int, Sprite*> _playerMap;

  bool _isMapBuilt;

public:
  GameScene(TextureManager &tm) : _texManager(tm), _isMapBuilt(false) {}

  void onEnter() override;

  SceneState update(const std::vector<Zappy::Event> &events,
                    const Zappy::GameState &gameState,
                    const std::vector<Zappy::NetworkEvent> &netEvents,
                    float deltaTime) override;

  void draw(Shader &) override;

  void onExit() override;

private:
  void buildMap(const Zappy::GameState &gameState);

  void spawnPlayer3D(int id, const Zappy::Player& p, Texture& tex, float offX, float offZ);

  void removePlayer3D(int id);
};
} // namespace Zappy 