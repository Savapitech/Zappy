#pragma once

#include "Core/Shader/Shader.hpp"
#include "Font/FontManager.hpp"
#include "GameState.hpp"
#include "Text/Text.hpp"
#include "Utils/math.hpp"

#include <memory>
#include <string>
#include <vector>

namespace Zappy {

class Hud {
public:
  explicit Hud(FontManager &fontManager);
  ~Hud();

  void onEnter();
  void update(const Zappy::GameState &gameState);
  void draw();
  void onExit();

private:
  struct TeamChip {
    std::string name;
    std::unique_ptr<Text> text;
  };

  void rebuildTeamChips(const std::vector<std::string> &teamNames);
  Zappy::Math::vec3 teamColor(const std::string &teamName) const;

  FontManager &_fontManager;

  unsigned int _panelVAO = 0;
  unsigned int _panelVBO = 0;
  std::unique_ptr<Shader> _panelShader;
  std::unique_ptr<Shader> _textShader;

  std::unique_ptr<Text> _infoText;
  std::unique_ptr<Text> _levelText;
  std::vector<TeamChip> _teamChips;

  std::string _lastInfo;
  std::string _lastLevels;
  std::string _lastTeamSignature;

  static constexpr float BAR_HEIGHT = 88.0f;
};

} // namespace Zappy
