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
            Font &get(const std::string &filepath) {
            auto it = _fonts.find(filepath);
            if (it == _fonts.end()) {
                _fonts[filepath] = std::make_unique<Font>(filepath);
                return *_fonts[filepath];
            }
            return *(it->second);
      }
    };
} // namespace Zappy
