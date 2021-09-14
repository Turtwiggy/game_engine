#pragma once

// engine headers
#include "engine/opengl/shader.hpp"

// other project headers
#include <glm/glm.hpp>

// c++ headers
#include <vector>

namespace engine {

namespace triangle_renderer {

// data required per triangle
struct TriangleDescriptor
{
  glm::vec2 point_0 = { 0, 0 };
  glm::vec2 point_1 = { 0, 0 };
  glm::vec2 point_2 = { 0, 0 };
  glm::vec4 point_0_colour = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
  glm::vec4 point_1_colour = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
  glm::vec4 point_2_colour = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
  int tex_slot = 0;
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

class TriangleRenderer
{
public:
  static void draw_triangle(const TriangleDescriptor& r, Shader& s);

  static void init();
  static void shutdown();

  static void end_batch();
  static void flush(Shader& shader); // submit for a drawcall
  static void begin_batch();

  static void reset_quad_vert_count();
  static void end_frame();

  static int draw_calls();

private:
  static constexpr int max_quad = 10000;
  static constexpr int max_quad_vert_count = max_quad * 3;
  static constexpr int max_quad_index_count = max_quad * 3;
};

} // namespace triangle_renderer

} // namespace engine