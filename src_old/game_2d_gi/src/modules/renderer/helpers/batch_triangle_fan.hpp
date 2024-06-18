#pragma once

// engine headers
#include "opengl/shader.hpp"

namespace engine {

namespace triangle_fan_renderer {

// data for vert.glsl
struct Vertex
{
  glm::vec4 pos;
  glm::mat4 model;
};

class TriangleFanRenderer
{
public:
  static void add_point_to_fan(const glm::ivec2 point, Shader& s);

  static void init();
  static void shutdown();

  static void end_batch();
  static void flush(Shader& s); // submit for a drawcall
  static void begin_batch();

  static void reset_quad_vert_count();
  static void end_frame();

  static int draw_calls();

private:
  static constexpr int max_quad = 5000;
  static constexpr int max_quad_vert_count = max_quad * 1;
  static constexpr int max_quad_index_count = max_quad * 1;
};

} // namespace triangle_fan_renderer

} // namespace engine