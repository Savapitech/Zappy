#pragma once
#include "Buttons/IButton.hpp"
#include "Sprite/Sprite.hpp"
#include "IScene/IScene.hpp"
#include <functional>
#include <memory>

namespace Zappy
{
    class Button : public IButton {
        private:
            std::unique_ptr<Sprite> _sprite;
            std::function<void()> _function;
            float _width;
            float _height;
            float _x;
            float _y;
            bool _hovered;
        public:
            Button(Texture& texture, float x, float y, float width, float height, std::function<void()> function);
            void draw(Shader &shader);
            void setPosition(float x, float y);
            void update(const std::vector<Zappy::Event> &events);
    };

} // namespace Z
