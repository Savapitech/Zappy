#pragma once

#include "Font/Font.hpp"
#include "Core/Shader/Shader.hpp"
#include "Utils/math.hpp"
#include <string>


namespace Zappy
{
    class Text {
        private:
            unsigned int _VAO;
            unsigned int _VBO;
            Font &_font;
            std::string _content;
            void updateGeo();

        public:
            Text(Font &font, const std::string &text, float x, float y);
            ~Text();
            void setString(const std::string &text);
            void setScale(float newScale);
            float getWidth() const;
            void setPosition(float x, float y);
            void draw(Shader &shader, const Zappy::Math::mat4 &projection) const;
            Zappy::Math::vec2 position;
            Zappy::Math::vec3 color;
            float scale;
            float alpha;
    };
} // namespace Zappy
