#pragma once

// other library headers
#include "glm/glm.hpp"

namespace fightingengine {

struct Vertex
{
  glm::vec3 Position = glm::vec3(0.0, 0.0, 0.0);
  glm::vec3 Normal = glm::vec3(0.0, 0.0, 0.0);
  glm::vec2 TexCoords = glm::vec2(0.0, 0.0);
  // glm::vec3 Tangent;
  // glm::vec3 Bitangent;

  // ColourVec4f Colour;
};

} // namespace fightingengine
