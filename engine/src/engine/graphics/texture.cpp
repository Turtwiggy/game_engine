
// header
#include "engine/graphics/texture.hpp"

// c lib headers
#include <assert.h>

// c++ standard library headers
#include <iostream>

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

} // namespace fightingengine
