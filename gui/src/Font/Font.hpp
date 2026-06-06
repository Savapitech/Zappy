#pragma once

#include <string>


#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif


#include "Utils/stb_truetype.h"

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif


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
            Font(const std::string &filepath, int atlasWidth = 512, int atlasHeight = 512, float fontSize = 32.0f);
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
