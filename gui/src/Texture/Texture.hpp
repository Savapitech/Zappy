#pragma once

#include <string>

namespace Zappy {
    class Texture {
    private:
        unsigned int _id;
        int _width;
        int _height;
        int _channels;

    public:
        Texture(const std::string& filepath);
        ~Texture();

        Texture(const Texture&) = delete;
        Texture& operator=(const Texture&) = delete;

        void bind() const;
        void unbind() const;

        inline unsigned int getId() const { return _id; }
        inline int getWidth() const { return _width; }
        inline int getHeight() const { return _height; }
    };
}
