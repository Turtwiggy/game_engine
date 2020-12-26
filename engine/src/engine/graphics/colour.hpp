#pragma once

// other library headers
#include <glm/glm.hpp>

namespace fightingengine {

/*
    A wrapper around the glm::vec4 for colour
*/
struct ColourVec4f
{
  ColourVec4f() = default;
  ColourVec4f(glm::vec4 c)
    : colour(c)
  {}

  glm::vec4 colour = glm::vec4(1.0f, (110 / 255.0f), (199 / 255.0f), 1.0f);
};

} // namespace fightingengine
