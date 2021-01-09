#pragma once

// other library headers
#include "glm/glm.hpp"

namespace fightingengine {

struct Vertex
{
  glm::vec3 position = glm::vec3(0.0, 0.0, 0.0);
  glm::vec3 normal = glm::vec3(0.0, 0.0, 0.0);
  glm::vec2 tex_coords = glm::vec2(0.0, 0.0);

  // ColourVec4f Colour;
};

} // namespace fightingengine
