#pragma once

// other lib headers
#include <glm/glm.hpp>
#if !defined(STB_IMAGE_IMPLEMENTATION)
#define STB_IMAGE_IMPLEMENTATION
#endif

// c++ standard library headers
#include <string>
#include <vector>

namespace engine {

struct StbLoadedTexture
{
  int width;
  int height;
  int nr_components;
  int texture_unit;
  std::string path;
  unsigned char* data;
};

StbLoadedTexture
load_texture(const int textureUnit, const std::string& path);

[[nodiscard]] std::vector<unsigned int>
load_textures_threaded(std::vector<std::pair<int, std::string>>& textures_to_load);

[[nodiscard]] unsigned int
bind_stb_loaded_texture(StbLoadedTexture& texture);

void
bind_tex(const int id, const int unit = -1);

void
unbind_tex();

unsigned int
create_texture(glm::ivec2 size, int tex_slot, unsigned int framebuffer_id);

void
update_bound_texture_size(glm::ivec2 size);

} // namespace engine
