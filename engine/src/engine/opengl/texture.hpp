#pragma once

// other lib headers
#include <glm/glm.hpp>

// c++ standard library headers
#include <string>

namespace engine {

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

[[nodiscard]] unsigned int
bind_stb_loaded_texture(StbLoadedTexture& texture);

// Texture util functions
void
bind_tex(const int id, const int unit = -1);

void
unbind_tex();

unsigned int
create_texture(glm::ivec2 size, int tex_slot, unsigned int framebuffer_id);

void
update_bound_texture_size(glm::ivec2 size);

} // namespace engine
