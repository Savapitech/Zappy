#include "Buttons/Button.hpp"
#include <iostream>
namespace Zappy
{
Button::Button(Texture &texture, float x, float y, float width, float height, std::function<void()> function, WindowSize &ws) : _function(function), _width(width), _height(height), _x(x), _y(y), _hovered(false), _ws(ws)
{
    _sprite = std::make_unique<Sprite>(texture);
    _sprite->setPosition(x, y);
    _sprite->scale = Zappy::Math::vec3(width, height, 1.0f);


}

void Button::draw(Shader &shader)
{
    if (_sprite) {
        Zappy::Math::mat4 orthoProjection = Zappy::Math::ortho(0.0f, _ws.width, _ws.height, 0.0f, -1.0f, 1.0f);
        Zappy::Math::mat4 view;
        _sprite->draw(shader, view, orthoProjection);
    }
}

void Button::setPosition(float x, float y)
{
    _x = x;
    _y = y;
    if (_sprite) {
        _sprite->setPosition(x, y);
    }
}

void Button::update(const std::vector<Zappy::Event> &events)
{
    for (const auto &event : events)
    {
        if (event.type == EventType::MouseMoved){
            if (event.mouseX >= _x - _width / 2 && event.mouseX <= _x + _width / 2 && event.mouseY >= _y - _height / 2 && event.mouseY <= _y + _height / 2){
                _hovered = true;
            } else {
                _hovered = false;
            }
        }
        if (event.type == EventType::MousePressed && event.button == 1) {
            if (_hovered) {
                _function();
            }
        }
    }
}
}// namespace Zappy
