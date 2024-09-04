#pragma once

#include "glm/glm.hpp"

namespace game2d {

struct SetPositionAtDynamicTarget
{
  glm::vec2 offset{ 0, 0 };
};

struct SetRotationAsDynamicTarget
{
  float offset = 0.0f;
};

struct LerpingToTarget
{
  glm::vec2 a{ 0.0f, 0.0f };
  glm::vec2 b{ 0.0f, 0.0f };
  float t = 0.0f;
};

} // namespace game2d