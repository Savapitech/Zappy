#pragma once

#include "../Window/Window.hpp"
#include "SceneManager/SceneManager.hpp"
#include "Texture/Texture.hpp"
#include "Texture/TextureManager.hpp"
#include "Network/NetworkManager.hpp"
#include "Core/Shader/Shader.hpp"
#include "Sprite/Sprite.hpp"

#include <memory>

#define FPS60 16

namespace Zappy {
class Core {
private:
  Zappy::SceneManager _sceneManager;
  std::unique_ptr<Zappy::Shader> _defaultShader;
  bool _isRunning;
  Zappy::Window _window;
  Zappy::NetworkManager _networkManager;
  bool isConnected;

  std::string _ip;
  int _port;

public:
  Core();
  ~Core();

  void init(const std::string& ip, int port);
  void run(void);
};
} // namespace Zappy
