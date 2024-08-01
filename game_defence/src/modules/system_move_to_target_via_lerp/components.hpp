#pragma once

#include "glm/glm.hpp"

namespace game2d {

struct SetPositionAtDynamicTarget
{
  bool placeholder = true;
};

struct LerpingToTarget
{
  glm::vec2 a;
  glm::vec2 b;
  float t;
};

} // namespace game2d