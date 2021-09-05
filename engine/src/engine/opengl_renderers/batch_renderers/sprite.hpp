#pragma once

// engine headers
#include "engine/opengl/shader.hpp"

// other project headers
#include <glm/glm.hpp>

// c++ headers
#include <vector>

namespace fightingengine {

namespace sprite_renderer {

// data required per sprite
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
  glm::vec4 pos_and_uv;
  glm::vec4 colour;
  glm::vec2 sprite_offset;
  float tex_slot;
  glm::mat4 model;
};

class SpriteBatchRenderer
{
public:
  static void draw_sprite(const RenderDescriptor& r);

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
    Shader("2d_game/shaders/2d_instanced.vert", "2d_game/shaders/2d_instanced.frag");
};

} // namespace sprite_renderer

} // namespace fightingengine