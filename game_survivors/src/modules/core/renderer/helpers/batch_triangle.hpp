#pragma once

// engine headers
#include "engine/colour/colour.hpp"
#include "engine/opengl/shader.hpp"

// other project headers
#include <glm/fwd.hpp>

namespace engine {

namespace tri_renderer {

// data required per triangle
struct TriangleDescriptor
{
  glm::vec2 point_0 = { 0, 0 };
  glm::vec2 point_1 = { 0, 0 };
  glm::vec2 point_2 = { 0, 0 };
  LinearColour point_0_colour;
  LinearColour point_1_colour;
  LinearColour point_2_colour;
};

// data for vert.glsl
struct TriVertex
{
  glm::vec4 pos_and_uv;
  glm::vec4 colour;
  glm::mat4 model;
};

struct RenderData
{
  unsigned int VAO = 0;
  unsigned int VBO = 0;
  unsigned int EBO = 0;

  uint32_t index_count = 0;
  int tri_vertex = 0;

  TriVertex* buffer;
  TriVertex* buffer_ptr;

  int draw_calls = 0; // stats
};

class TriangleRenderer
{
public:
  RenderData data;

public:
  void draw_sprite(const TriangleDescriptor& r, const Shader& s);

  void init();
  void shutdown();

  void end_batch();
  void flush(const Shader& shader); // submit for a drawcall
  void begin_batch();

  void reset_vert_count();
  void end_frame();

  int draw_calls() const;

private:
  constexpr static int max_tri = 5'000;
  constexpr static int max_tri_vert_count = max_tri * 3;
  constexpr static int max_tri_index_count = max_tri * 3;
};

} // namespace quad_renderer

} // namespace engine