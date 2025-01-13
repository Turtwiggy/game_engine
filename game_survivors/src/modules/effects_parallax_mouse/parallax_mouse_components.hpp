#pragma once

#include <glm/glm.hpp>

namespace game2d {

struct ParallaxMouseComponent
{
  glm::vec2 translation{ 0, 0 };
  glm::vec2 rotation{ 0, 0 };
};

} // namespace game2d