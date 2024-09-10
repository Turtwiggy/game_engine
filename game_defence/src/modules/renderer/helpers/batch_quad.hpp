#pragma once

// engine headers
#include "colour/colour.hpp"
#include "opengl/shader.hpp"

// other project headers
#include <glm/glm.hpp>

namespace engine {

namespace quad_renderer {

// data required per sprite
struct RenderDescriptor
{
  glm::vec2 pos_tl = { 0, 0 };
  glm::vec2 size = { 1, 1 };
  float angle_radians = 0.0f;
  LinearColour colour;

  // sprite info
  glm::ivec2 sprite_offset = { 0, 0 }; // e.g. pos 0, 5
  glm::ivec2 sprite_width = { 0, 0 };  // e.g. 2, 2
  glm::ivec2 sprites_max = { 0, 0 };   // e.g. 22

  // the spritesheet texture unit this belongs to
  int tex_unit = 0;
};

// data for vert.glsl
struct Vertex
{
  glm::vec4 pos_and_uv;
  glm::vec4 colour;
  glm::vec4 sprite_offset;
  glm::vec4 sprite_width_and_max;
  float tex_unit;
  glm::mat4 model;
};

struct RenderData
{
  unsigned int VAO = 0;
  unsigned int VBO = 0;
  unsigned int EBO = 0;

  unsigned int TBO = 0;
  unsigned int TEX = 0;
  unsigned int tex_unit = 0;

  uint32_t index_count = 0;
  int quad_vertex = 0;

  Vertex* buffer;
  Vertex* buffer_ptr;

  int draw_calls = 0; // stats
};

class QuadRenderer
{
public:
  RenderData data;

public:
  void draw_sprite(const RenderDescriptor& r, const Shader& s);

  void init();
  void shutdown();

  void end_batch();
  void flush(const Shader& shader); // submit for a drawcall
  void begin_batch();

  void reset_quad_vert_count();
  void end_frame();

  int draw_calls();

private:
  constexpr static int N_MAX_CIRCLES = 100;
  constexpr static int max_quad = 2000;
  constexpr static int max_quad_vert_count = max_quad * 4;
  constexpr static int max_quad_index_count = max_quad * 6;
};

} // namespace quad_renderer

} // namespace engine