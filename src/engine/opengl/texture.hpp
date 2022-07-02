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

struct SRGBTexture
{
  int width;
  int height;
  int nr_components;
  int texture_unit;
  std::string path;
  unsigned char* data; // 0-255
};

// no pow 2.2 this is bad
// no approximations this is also bad
struct LinearTexture
{
  int width;
  int height;
  int nr_components;
  int texture_unit;
  std::string path;
  float* data; // linear colour
};

void
bind_tex(const int id);

void
unbind_tex();

// [[nodiscard]] LinearTexture
// load_texture_srgb(const int tex_unit, const std::string& path);

// [[nodiscard]] LinearTexture
// load_texture_linear(const int tex_unit, const std::string& path);

[[nodiscard]] std::vector<unsigned int>
load_textures_threaded(std::vector<std::pair<int, std::string>>& textures_to_load);

[[nodiscard]] unsigned int
bind_linear_texture(const LinearTexture& texture);

void
update_bound_texture_size(const glm::ivec2 size);

void
new_texture_to_fbo(unsigned int& out_fbo_id, int& out_tex_id, const int tex_unit, const glm::ivec2 size);

} // namespace engine
