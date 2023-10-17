#pragma once

// engine headers
#include "colour/colour.hpp"
#include "opengl/shader.hpp"

// other project headers
#include <glm/glm.hpp>

// c++ headers
#include <vector>

namespace engine {

namespace quad_renderer {

// data required per sprite
struct RenderDescriptor
{
  glm::ivec2 pos_tl = { 0, 0 };
  glm::ivec2 size = { 1, 1 };
  float angle_radians = 0.0f;
  LinearColour colour;
  glm::ivec4 sprite_offset_and_spritesheet = { 0, 0, 0, 0 };
  int tex_unit = 0;
};

// data for vert.glsl
struct Vertex
{
  glm::vec4 pos_and_uv;
  glm::vec4 colour;
  glm::vec4 sprite_offset_and_spritesheet;
  float tex_unit;
  glm::mat4 model;
};

class QuadRenderer
{
public:
  static void draw_sprite(const RenderDescriptor& r, const Shader& s);

  static void init();
  static void shutdown();

  static void end_batch();
  static void flush(const Shader& shader); // submit for a drawcall
  static void begin_batch();

  static void reset_quad_vert_count();
  static void end_frame();

  static int draw_calls();

private:
  static constexpr int max_quad = 10000;
  static constexpr int max_quad_vert_count = max_quad * 4;
  static constexpr int max_quad_index_count = max_quad * 6;
};

} // namespace quad_renderer

} // namespace engine