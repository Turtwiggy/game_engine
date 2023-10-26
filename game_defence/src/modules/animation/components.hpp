#pragma once

#include <glm/glm.hpp>

namespace game2d {

struct SetTransformAngleToVelocity
{
  bool placeholder = true;
};

struct WiggleUpAndDown
{
  glm::ivec2 base_position;
  float time = 0.0f;
  float offset = 0.0f;
};

} // namespace game2d