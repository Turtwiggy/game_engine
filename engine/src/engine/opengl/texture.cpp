
// header
#include "engine/opengl/texture.hpp"

// c lib headers
#include <assert.h>

// c++ standard library headers
#include <iostream>

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace fightingengine {

void
Texture2D::generate(unsigned int width,
                    unsigned int height,
                    GLenum internalFormat,
                    GLenum format,
                    GLenum type,
                    void* data)
{
  glGenTextures(1, &this->id);

  Width = width;
  Height = height;
  InternalFormat = internalFormat;
  Format = format;
  Type = type;

  assert(Target == GL_TEXTURE_2D);
  bind();

  glTexImage2D(Target, 0, internalFormat, width, height, 0, format, type, data);
  glTexParameteri(Target, GL_TEXTURE_MIN_FILTER, FilterMin);
  glTexParameteri(Target, GL_TEXTURE_MAG_FILTER, FilterMax);
  glTexParameteri(Target, GL_TEXTURE_WRAP_S, WrapS);
  glTexParameteri(Target, GL_TEXTURE_WRAP_T, WrapT);
  if (Mipmapping)
    glGenerateMipmap(Target);

  unbind();
}

void
Texture2D::bind(int unit) const
{
  if (unit >= 0)
    glActiveTexture(GL_TEXTURE0 + unit);
  glBindTexture(Target, this->id);
}

void
Texture2D::unbind()
{
  glBindTexture(Target, 0);
}

Texture2D
load_texture(std::string full_path, GLenum target, GLenum internalFormat, bool srgb)
{
  // create texture object
  Texture2D texture;
  texture.Target = target;
  texture.InternalFormat = internalFormat;
  if (texture.InternalFormat == GL_RGB || texture.InternalFormat == GL_SRGB)
    texture.InternalFormat = srgb ? GL_SRGB : GL_RGB;
  if (texture.InternalFormat == GL_RGBA || texture.InternalFormat == GL_SRGB_ALPHA)
    texture.InternalFormat = srgb ? GL_SRGB_ALPHA : GL_RGBA;

  // flip textures on their y coordinate while loading
  // stbi_set_flip_vertically_on_load(false);

  // load image
  int width, height, nrChannels;
  void* data = stbi_load(full_path.c_str(), &width, &height, &nrChannels, 0);

  if (data) {
    GLenum format;
    if (nrChannels == 1)
      format = GL_RED;
    else if (nrChannels == 3)
      format = GL_RGB;
    else if (nrChannels == 4)
      format = GL_RGBA;

    if (target == GL_TEXTURE_2D) {
      texture.WrapR = GL_REPEAT;
      texture.WrapS = format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT;
      texture.WrapT = format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT;
      texture.FilterMin = GL_LINEAR_MIPMAP_LINEAR;
      texture.FilterMax = GL_LINEAR;

      texture.generate(width, height, texture.InternalFormat, format, GL_UNSIGNED_BYTE, data);
    }
  } else {
    printf("FAILED TO LOAD TEXTURE: %s", full_path.c_str());
  }

  // and finally free image data
  stbi_image_free(data);

  return texture;
}

} // namespace fightingengine
