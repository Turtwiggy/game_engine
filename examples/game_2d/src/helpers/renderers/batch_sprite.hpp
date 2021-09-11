#pragma once

// engine headers
#include "engine/opengl/shader.hpp"

// other project headers
#include <glm/glm.hpp>

// c++ headers
#include <vector>

namespace engine {

namespace sprite_renderer {

// data required per sprite
struct RenderDescriptor
{
  glm::ivec2 pos_tl = { 0, 0 };
  glm::ivec2 size = { 100, 100 };
  glm::vec4 colour = { 1.0f, 0.0f, 0.0f, 1.0f };
  float angle_radians = 0.0f;
  glm::ivec2 sprite_offset = { 0, 0 }; // e.g. { 5, 8 }; for spritesheet
  int tex_slot = 0;
  int z_index = 0;
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
  static void draw_sprite(const RenderDescriptor& r, Shader& s);

  static void init();
  static void shutdown();

  static void end_batch();
  static void flush(Shader& shader); // submit for a drawcall
  static void begin_batch();

  static void reset_quad_vert_count();
  static void end_frame();

private:
  static constexpr int max_quad = 10000;
  static constexpr int max_quad_vert_count = max_quad * 4;
  static constexpr int max_quad_index_count = max_quad * 6;
};

} // namespace sprite_renderer

} // namespace engine