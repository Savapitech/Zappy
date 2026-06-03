#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "Texture.hpp"

namespace Zappy {
class TextureManager {
private:
  std::unordered_map<std::string, std::unique_ptr<Texture>> _textures;

public:
  Texture &get(const std::string &filepath) {
    auto it = _textures.find(filepath);

    if (it == _textures.end()) {
      _textures[filepath] = std::make_unique<Texture>(filepath);
      return *_textures[filepath];
    }

    return *(it->second);
  }
};
} // namespace Zappy
