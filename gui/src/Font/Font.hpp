#pragma once
#include <string>
#include "Utils/stb_truetype.h"

namespace Zappy
{
    class Font {
        private:
            unsigned int _id;
            int _atlasWidth;
            int _atlasHeight;
            float _fontSize;
            stbtt_bakedchar _charData[96];

        public:
            Font(const std::string &filepath, int atlasWidth, int atlasHeight, float fontSize);
            ~Font();
            Font(const Font &) = delete;
            Font &operator=(const Font &) = delete;
            void bind() const;
            void unbind() const;
            inline unsigned int getId() const { return _id;}
            inline const stbtt_bakedchar *getCharData() const { return _charData;}
            inline int getAtlasWidth() const {return _atlasWidth;}
            inline int getAtlasHeight() const {return _atlasHeight;}
    };
    
} // namespace Zappy
