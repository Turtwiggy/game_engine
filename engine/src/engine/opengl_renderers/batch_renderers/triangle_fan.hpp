#pragma once

// engine headers
#include "engine/opengl/shader.hpp"

namespace fightingengine {

namespace triangle_fan_renderer {

// data required per point
struct RenderDescriptor
{
  glm::vec2 pos_tl;
  glm::vec2 size;
  glm::vec4 colour;
  float angle_radians;
  glm::ivec2 sprite_offset; // e.g. { 5, 8 }; for spritesheet
  float tex_slot;
};

// data for vert.glsl
struct Vertex
{
  glm::vec4 pos;
  glm::mat4 model;
};

class TriangleFanRenderer
{
public:
  static void add_point_to_fan(Shader& shader, const glm::ivec2 point);

public:
  static void init();
  static void shutdown();

  static void end_batch();
  static void flush(Shader& shader); // submit for a drawcall
  static void begin_batch();

  static void reset_quad_vert_count();
  static void end_frame();

private:
  static constexpr int max_quad = 5000;
  static constexpr int max_quad_vert_count = max_quad * 4;
  static constexpr int max_quad_index_count = max_quad * 6;
  static inline Shader shader =
    fightingengine::Shader("2d_game/shaders/2d_basic_with_proj.vert", "2d_game/shaders/2d_colour.frag");
};

} // namespace triangle_fan_renderer

} // namespace fightingengine