#pragma once

// other lib headers
#include "engine/deps/opengl.hpp"
#if !defined(STB_IMAGE_IMPLEMENTATION)
#define STB_IMAGE_IMPLEMENTATION
#endif
#include "engine/opengl/framebuffer.hpp"
#include <glm/fwd.hpp>

// c++ standard library headers
#include <string>
#include <vector>

namespace engine {

struct TextureFiltering
{
  // min: filtering mode if texture pixels < screen pixels
  // mag: filtering mode if texture pixels > screen pixels
  int texture_wrap_s = 0;
  int texture_wrap_t = 0;
  int texture_min_filter = 0;
  int texture_mag_filter = 0;
};

struct SRGBTexture
{
  int width = 0;
  int height = 0;
  int nr_components = 0;
  int texture_unit = 0;
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
  TextureFiltering filtering;
};

// known before bind
// i.e. chosen by user
struct TextureUnit
{
  int unit = 0;

  TextureUnit() = default;
  TextureUnit(int unit)
    : unit(unit) {};
};

// known after bind
// i.e. chosen by opengl
struct TextureId
{
  int id = 0;
};

struct Texture
{
  std::string path;
  std::string spritesheet_path;
  TextureUnit tex_unit;
  TextureId tex_id;
  glm::ivec2 size{ 0, 0 };

  Texture() = default;
  Texture(const std::string& p, const std::string& sp)
    : path(p)
    , spritesheet_path(sp) {};
};

void
bind_tex(const int id);

void
unbind_tex();

// [[nodiscard]] std::vector<unsigned int>
// load_textures(const std::vector<std::pair<int, std::string>>& textures_to_load);

// [[nodiscard]] std::vector<unsigned int>
// load_textures_threaded(const std::vector<std::pair<int, std::string>>& textures_to_load);

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
new_texture_to_fbo(const int tex_unit, const glm::ivec2& size, const TextureFiltering& f, const int n_colour_buffers = 1);

} // namespace engine
