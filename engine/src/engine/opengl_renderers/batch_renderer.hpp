#pragma once

// engine headers
#include "engine/opengl/shader.hpp"

// other lib headers
#include <glm/glm.hpp>

namespace fightingengine {

template<class Vertex>
struct RenderData
{
  unsigned int VAO = 0;
  unsigned int VBO = 0;
  unsigned int EBO = 0;

  uint32_t index_count = 0;
  int quad_vertex = 0;

  Vertex* buffer;
  Vertex* buffer_ptr;

  int draw_calls = 0; // stats
};

} // namespace fightingengine