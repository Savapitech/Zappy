#include "Hud.hpp"

#include "IScene/IScene.hpp"
#include "Utils/OpenGL.hpp"

#include <array>
#include <map>

namespace Zappy {

Hud::Hud(FontManager &fontManager) : _fontManager(fontManager) {}

Hud::~Hud() { onExit(); }

void Hud::onEnter() {
  _panelShader = std::make_unique<Shader>("gui/src/Core/Shader/panel.vert",
                                          "gui/src/Core/Shader/panel.frag");
  _textShader = std::make_unique<Shader>("gui/src/Core/Shader/text.vert",
                                         "gui/src/Core/Shader/text.frag");

  Font &font = _fontManager.get("gui/assets/fonts/mainTitle.otf", 26.0f, 512);

  _infoText = std::make_unique<Text>(font, "", 30.0f, 36.0f);
  _infoText->color = Zappy::Math::vec3(1.0f, 1.0f, 1.0f);

  _levelText = std::make_unique<Text>(font, "", 30.0f, 72.0f);
  _levelText->color = Zappy::Math::vec3(0.75f, 0.85f, 1.0f);

  std::array<float, 12> quad = {0.0f,  0.0f,       WIDTH, 0.0f,
                                WIDTH, BAR_HEIGHT, 0.0f,  0.0f,
                                WIDTH, BAR_HEIGHT, 0.0f,  BAR_HEIGHT};
  glGenVertexArrays(1, &_panelVAO);
  glGenBuffers(1, &_panelVBO);
  glBindVertexArray(_panelVAO);
  glBindBuffer(GL_ARRAY_BUFFER, _panelVBO);
  glBufferData(GL_ARRAY_BUFFER, quad.size() * sizeof(float), quad.data(),
               GL_STATIC_DRAW);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

Zappy::Math::vec3 Hud::teamColor(const std::string &teamName) const {
  unsigned int hash = 2166136261u;
  for (char c : teamName) {
    hash ^= static_cast<unsigned char>(c);
    hash *= 16777619u;
  }
  float r = 0.45f + ((hash >> 0) & 0xFF) / 255.0f * 0.55f;
  float g = 0.45f + ((hash >> 8) & 0xFF) / 255.0f * 0.55f;
  float b = 0.45f + ((hash >> 16) & 0xFF) / 255.0f * 0.55f;
  return Zappy::Math::vec3(r, g, b);
}

void Hud::rebuildTeamChips(const std::vector<std::string> &teamNames) {
  _teamChips.clear();
  Font &font = _fontManager.get("gui/assets/fonts/mainTitle.otf", 26.0f, 512);
  for (const auto &name : teamNames) {
    TeamChip chip;
    chip.name = name;
    chip.text = std::make_unique<Text>(font, "", 0.0f, 72.0f);
    chip.text->color = teamColor(name);
    _teamChips.push_back(std::move(chip));
  }
}

void Hud::update(const Zappy::GameState &gameState) {
  if (!gameState.map.isInitialized)
    return;

  std::map<std::string, int> perTeam;
  std::array<int, 9> perLevel = {0, 0, 0, 0, 0, 0, 0, 0, 0};
  for (const auto &name : gameState.map.teamNames)
    perTeam[name] = 0;
  for (const auto &[id, player] : gameState.players) {
    (void)id;
    perTeam[player.team]++;
    if (player.level >= 1 && player.level <= 8)
      perLevel[player.level]++;
  }

  std::string info = "MAP " + std::to_string(gameState.map.width) + "x" +
                     std::to_string(gameState.map.height) + "    FREQ " +
                     std::to_string(gameState.map.timeUnit) + "/u    PLAYERS " +
                     std::to_string(gameState.players.size()) + "    EGGS " +
                     std::to_string(gameState.eggs.size());
  if (info != _lastInfo) {
    _infoText->setString(info);
    _lastInfo = info;
  }

  std::string levels = "LEVELS";
  for (int lvl = 1; lvl <= 8; lvl++)
    levels += "   " + std::to_string(lvl) + ":" + std::to_string(perLevel[lvl]);
  if (levels != _lastLevels) {
    _levelText->setString(levels);
    _lastLevels = levels;
  }

  std::string signature;
  for (const auto &[name, count] : perTeam)
    signature += name + ",";
  if (signature != _lastTeamSignature) {
    std::vector<std::string> names;
    for (const auto &[name, count] : perTeam) {
      (void)count;
      names.push_back(name);
    }
    rebuildTeamChips(names);
    _lastTeamSignature = signature;
  }

  float cursorX = 30.0f + _levelText->getWidth() + 80.0f;
  for (auto &chip : _teamChips) {
    std::string label = chip.name + " " + std::to_string(perTeam[chip.name]);
    chip.text->setString(label);
    chip.text->setPosition(cursorX, 72.0f);
    cursorX += chip.text->getWidth() + 40.0f;
  }
}

void Hud::draw() {
  if (!_panelShader || !_textShader || _panelVAO == 0)
    return;

  Zappy::Math::mat4 ortho =
      Zappy::Math::ortho(0.0f, WIDTH, HEIGHT, 0.0f, -1.0f, 1.0f);

  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  _panelShader->bind();
  _panelShader->setMat4("u_MVP", ortho);
  _panelShader->setVec3("u_Color", Zappy::Math::vec3(0.04f, 0.05f, 0.09f));
  _panelShader->setFloat("u_Alpha", 0.72f);
  glBindVertexArray(_panelVAO);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  glBindVertexArray(0);

  _infoText->draw(*_textShader, ortho);
  _levelText->draw(*_textShader, ortho);
  for (auto &chip : _teamChips)
    chip.text->draw(*_textShader, ortho);

  glDisable(GL_BLEND);
  glEnable(GL_DEPTH_TEST);
}

void Hud::onExit() {
  if (_panelVBO != 0) {
    glDeleteBuffers(1, &_panelVBO);
    _panelVBO = 0;
  }
  if (_panelVAO != 0) {
    glDeleteVertexArrays(1, &_panelVAO);
    _panelVAO = 0;
  }
  _infoText.reset();
  _levelText.reset();
  _teamChips.clear();
  _panelShader.reset();
  _textShader.reset();
  _lastInfo.clear();
  _lastLevels.clear();
  _lastTeamSignature.clear();
}

} // namespace Zappy
