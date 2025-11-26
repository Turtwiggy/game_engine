#pragma once

// other lib headers
#include "engine/deps/opengl.hpp"
#include <cstdint>
#if !defined(STB_IMAGE_IMPLEMENTATION)
#define STB_IMAGE_IMPLEMENTATION
#endif
#include "engine/opengl/framebuffer.hpp"
#include <glm/fwd.hpp>

// c++ standard library headers
#include <string>
#include <vector>

namespace engine {

enum class Filtering
{
  clamp_to_border = GL_CLAMP_TO_BORDER,
  clamp_to_edge = GL_CLAMP_TO_EDGE,
  repeat = GL_REPEAT,

  nearest = GL_NEAREST,
  linear = GL_LINEAR,
};

struct TextureFiltering
{
  // min: filtering mode if texture pixels < screen pixels
  // mag: filtering mode if texture pixels > screen pixels
  Filtering texture_wrap_s = Filtering::clamp_to_border;
  Filtering texture_wrap_t = Filtering::clamp_to_border;
  Filtering texture_min_filter = Filtering::linear;
  Filtering texture_mag_filter = Filtering::linear;
};

struct SRGBTexture
{
  int width = 0;
  int height = 0;
  int nr_components = 0;
  uint32_t texture_id = 0;
  uint32_t texture_unit = 0;
  std::string path;
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
  bool linear = true;

  Texture() = default;
  Texture(const std::string& p, const std::string& sp)
    : path(p)
    , spritesheet_path(sp) {};
};

void
bind_tex(const int id);

void
unbind_tex();

SRGBTexture
load_texture(std::string path, const uint32_t tex_unit, bool linear = true);

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
