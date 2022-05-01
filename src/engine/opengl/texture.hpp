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
load_texture(const int tex_unit, const std::string& path);

[[nodiscard]] std::vector<unsigned int>
load_textures_threaded(std::vector<std::pair<int, std::string>>& textures_to_load);

[[nodiscard]] unsigned int
bind_stb_loaded_texture(StbLoadedTexture& texture);

void
bind_tex(const unsigned int id);

void
unbind_tex();

unsigned int
create_texture(const glm::ivec2 size, const unsigned int tex_unit, const int nr_components, unsigned char* data);

void
update_bound_texture_size(glm::ivec2 size);

void
new_texture_to_fbo(unsigned int& out_fbo_id, unsigned int& out_tex_id, const int tex_unit, glm::ivec2 size);

} // namespace engine
