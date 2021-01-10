#pragma once

// c++ standard library headers
#include <string>

// other library headers
#include <GL/glew.h>

namespace fightingengine {

enum class TextureType
{
  DIFFUSE,
  SPECULAR,
  NORMAL,
  HEIGHT
};

class Texture2D
{
public:
  unsigned int id;
  TextureType type;
  std::string path;

  // OpenGl Texture Data
  GLenum Target = GL_TEXTURE_2D;   // what type of texture
  GLenum InternalFormat = GL_RGBA; // number of color components
  GLenum Format = GL_RGBA;         // the format each texel is stored in
  GLenum Type = GL_UNSIGNED_BYTE;
  GLenum FilterMin = GL_LINEAR_MIPMAP_LINEAR; // what filter method to use during minification
  GLenum FilterMax = GL_LINEAR;               // what filter method to use during magnification
  GLenum WrapS = GL_REPEAT;                   // wrapping method of the S coordinate
  GLenum WrapT = GL_REPEAT;                   // wrapping method of the T coordinate
  GLenum WrapR = GL_REPEAT;                   // wrapping method of the R coordinate
  bool Mipmapping = true;

  Texture2D() = default;

  void load_texture_from_file(const std::string& full_path);

  void bind(int unit = -1) const;
  void unbind();
};

} // namespace fightingengine
