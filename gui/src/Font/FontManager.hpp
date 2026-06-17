#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "Font.hpp"

namespace Zappy {
class FontManager {
private:
  std::unordered_map<std::string, std::unique_ptr<Font>> _fonts;

public:
  Font &get(const std::string &filepath, float fontSize = 32.0f,
            int atlasSize = 512) {
    std::string key = filepath + "_" + std::to_string(fontSize);
    auto it = _fonts.find(key);
    if (it == _fonts.end()) {
      _fonts[key] =
          std::make_unique<Font>(filepath, atlasSize, atlasSize, fontSize);
      return *_fonts[key];
    }
    return *(it->second);
  }
};
} // namespace Zappy
