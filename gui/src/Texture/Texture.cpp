#include "Texture.hpp"

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
#pragma GCC diagnostic ignored "-Wcast-align"
#endif

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif

#include "Utils/OpenGL.hpp"
#include <iostream>

#include "Logger.hpp"

namespace Zappy {

Texture::Texture(const std::string &filepath)
    : _id(0), _width(0), _height(0), _channels(0) {
  stbi_set_flip_vertically_on_load(true);

  unsigned char *data =
      stbi_load(filepath.c_str(), &_width, &_height, &_channels, 0);

  if (!data) {
    LOG_ERROR("Error while loading the texture" + filepath);
    return;
  }
  GLenum format = (_channels == 4) ? GL_RGBA : GL_RGB;

  glGenTextures(1, &_id);
  glBindTexture(GL_TEXTURE_2D, _id);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glTexImage2D(GL_TEXTURE_2D, 0, format, _width, _height, 0, format,
               GL_UNSIGNED_BYTE, data);

  stbi_image_free(data);
}

Texture::~Texture() {
  if (_id != 0) {
    glDeleteTextures(1, &_id);
  }
}

void Texture::bind() const { glBindTexture(GL_TEXTURE_2D, _id); }

void Texture::unbind() const { glBindTexture(GL_TEXTURE_2D, 0); }

} // namespace Zappy
