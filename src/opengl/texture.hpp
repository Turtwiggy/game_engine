#pragma once

// other lib headers
#if !defined(STB_IMAGE_IMPLEMENTATION)
#define STB_IMAGE_IMPLEMENTATION
#endif
#include "opengl/framebuffer.hpp"
#include <glm/glm.hpp>

// c++ standard library headers
#include <string>
#include <utility>
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
  std::vector<float> data; // linear colour 0-1
};

void
bind_tex(const int id);

void
unbind_tex();

[[nodiscard]] std::vector<unsigned int>
load_textures(const std::vector<std::pair<int, std::string>>& textures_to_load);

[[nodiscard]] std::vector<unsigned int>
load_textures_threaded(const std::vector<std::pair<int, std::string>>& textures_to_load);

[[nodiscard]] LinearTexture
load_texture_linear(const int tex_unit, const std::string& path);

[[nodiscard]] unsigned int
bind_linear_texture(const LinearTexture& texture);

void
update_bound_texture_size(const glm::ivec2 size);

struct FboResult
{
  FramebufferID out_fbo_id;
  std::vector<unsigned int> out_tex_ids;
};
FboResult
new_texture_to_fbo(const int tex_unit, const glm::ivec2& size, const int n_colour_buffers = 1);

} // namespace engine
