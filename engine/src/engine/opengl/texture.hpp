#pragma once

// c++ standard library headers
#include <string>

namespace fightingengine {

enum class TextureType
{
  DIFFUSE,
  SPECULAR,
  NORMAL,
  HEIGHT
};

struct StbLoadedTexture
{
  int width;
  int height;
  int nr_components;
  int texture_unit;
  std::string path;
  unsigned char* data;
};

// Note: this IS thread safe
StbLoadedTexture
load_texture(const int textureUnit, const std::string& path);

void
bind_stb_loaded_texture(StbLoadedTexture& texture);

// Texture util functions
void
bind_tex(const int id, const int unit = -1);

void
unbind_tex();

} // namespace fightingengine
