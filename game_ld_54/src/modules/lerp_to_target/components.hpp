#pragma once

#include "glm/glm.hpp"
#include <entt/entt.hpp>

namespace game2d {

//
// weapon components
//

struct HasTargetPositionComponent
{
  glm::vec2 position{ 0, 0 };
};

struct LerpToTargetComponent
{
  float speed = 45.0f;
};

} // namespace game2d