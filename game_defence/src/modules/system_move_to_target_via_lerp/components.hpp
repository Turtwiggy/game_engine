#pragma once

#include "glm/glm.hpp"

namespace game2d {

struct SetPositionAtDynamicTarget
{
  glm::vec2 offset{ 0, 0 };
};

struct LerpingToTarget
{
  glm::vec2 a;
  glm::vec2 b;
  float t;
};

} // namespace game2d