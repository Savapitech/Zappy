#include <fstream>
#include <vector>

#include "Font.hpp"
#include "Logger.hpp"
#include "Utils/OpenGL.hpp"

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif

namespace Zappy {
    Font::Font(const std::string &filepath, int atlasWidth, int atlasHeight, float fontSize) : _id(0), _atlasWidth(atlasWidth), _atlasHeight(atlasHeight), _fontSize(fontSize)
    {
        std::ifstream file(filepath, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            LOG_FATAL("Failed to load the font:" + filepath);
            return;
        }
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);
        std::vector<unsigned char> ttfBuffer(size);
        if (!file.read(reinterpret_cast<char*>(ttfBuffer.data()), size)) {
            LOG_ERROR("Error while reading the font file: " + filepath);
            return;
        }
        std::vector<unsigned char> tempBitMap(_atlasWidth * _atlasHeight);
        stbtt_BakeFontBitmap(ttfBuffer.data(), 0, _fontSize, tempBitMap.data(), _atlasWidth, _atlasHeight, 32, 96, _charData);
        glGenTextures(1, &_id);
        glBindTexture(GL_TEXTURE_2D, _id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, _atlasWidth, _atlasHeight, 0, GL_RED, GL_UNSIGNED_BYTE, tempBitMap.data());
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    }
    Font::~Font() {
        if (_id != 0){
            glDeleteTextures(1, &_id);
        }
    }
    void Font::bind() const { glBindTexture(GL_TEXTURE_2D, _id);}
    void Font::unbind() const { glBindTexture(GL_TEXTURE_2D, 0);}
};
