#define GL_GLEXT_PROTOTYPES 1
#include "Text.hpp"
#include <GL/gl.h>
#include <GL/glx.h>
#include <vector>

namespace Zappy {
    Text::Text(Font &font, const std::string &text, float x, float y) : _VAO(0), _VBO(0), _font(font), _content(text), position(x, y), color(1.0f, 1.0f, 1.0f), scale(1.0f)
    {
        glGenVertexArrays(1, &_VAO);
        glGenBuffers(1, &_VBO);
        glBindVertexArray(_VAO);
        glBindBuffer(GL_ARRAY_BUFFER, _VBO);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        if (!_content.empty())
            updateGeo();
    }
    Text::~Text() {
        if (_VAO != 0)
            glDeleteVertexArrays(1, &_VAO);
        if (_VBO != 0)
            glDeleteBuffers(1, &_VBO);
    }
    void Text::setString(const std::string &text) {
        if (_content != text){
            _content = text;
            updateGeo();
        }
    }
    void Text::updateGeo() 
    {
        std::vector<float> vertices;
        float x = position.x;
        float y = position.y;

        for (char c : _content) {
            if (c >= 32 && c < 127) {
                stbtt_aligned_quad quad;
                stbtt_GetBakedQuad(_font.getCharData(), _font.getAtlasWidth(), _font.getAtlasHeight(), c -32, &x, &y, &quad, 1);
                vertices.insert(vertices.end(), {
                    quad.x0 * scale, quad.y1 * scale, quad.s0, quad.s1,
                    quad.x1 * scale, quad.y0 * scale, quad.s1, quad.t0,
                    quad.x0 * scale, quad.y0 * scale, quad.s0, quad.t0,
                    quad.x0 * scale, quad.y1 * scale, quad.s0, quad.t1,
                    quad.x1 * scale, quad.y1 * scale, quad.s1, quad.t1,
                    quad.x1 * scale, quad.y0 * scale, quad.s1, quad.t0
                });
            }
        }
        glBindBuffer(GL_ARRAY_BUFFER, _VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    void Text::draw(Shader &shader, const Zappy::Math::mat4 &projection) const {
        if (_content.empty())
            return;
        shader.bind();
        shader.setMat4("u_projection", projection);
        shader.setVec3("u_TextColor", color);
        shader.setInt("u_TextTexture", 0);
        glActiveTexture(GL_TEXTURE0);
        _font.bind();
        glBindVertexArray(_VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6 * _content.length());
        glBindVertexArray(0);
        _font.unbind();
    }
}
