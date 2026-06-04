#define GL_GLEXT_PROTOTYPES 1

#include "Core.hpp"
#include "Logger.hpp"

#include "SceneManager/SceneManager.hpp"

#include <chrono>
#include <thread>

#include <GL/gl.h>
#include <GL/glx.h>

namespace Zappy {

Core::Core() : _isRunning(true) {}

Core::~Core() {}

void Core::init() {
  _window.open(WIDTH, HEIGHT, "Zappy");

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

  _defaultShader = std::make_unique<Zappy::Shader>(
      "gui/src/Core/Shader/vertex.vert", "gui/src/Core/Shader/fragment.frag");

  _sceneManager.changeScene(
      std::make_unique<MenuScene>(_sceneManager.getTextureManager()));
}

void Core::run() {
  const std::chrono::microseconds frameDelay(16666);

  auto lastTime = std::chrono::steady_clock::now();
  while (_isRunning) {
    auto timeStart = std::chrono::steady_clock::now();
    float deltaTime = std::chrono::duration<float>(timeStart - lastTime).count();
    lastTime = timeStart;
    const auto &events = _window.pollEvents();
    for (const auto &event : events) {
      if (event.type == Zappy::EventType::WindowClosed) {
        _isRunning = false;
      }
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    _sceneManager.update(events, deltaTime);
    _defaultShader->bind();
    _sceneManager.draw(*_defaultShader);
    _window.swapBuffers();

    auto timeEnd = std::chrono::steady_clock::now();

    auto timeTaken = std::chrono::duration_cast<std::chrono::microseconds>(
        timeEnd - timeStart);

    if (timeTaken < frameDelay) {
      std::this_thread::sleep_for(frameDelay - timeTaken);
    }
  }
}
} // namespace Zappy