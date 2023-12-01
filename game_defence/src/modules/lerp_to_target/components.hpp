#pragma once

#include "glm/glm.hpp"
#include <entt/entt.hpp>

namespace game2d {

//
// weapon components
//

struct HasTargetPositionComponent
{
  glm::ivec2 position{ 0, 0 };
};

struct LerpToTargetComponent
{
  float speed = 150.0f;
};

} // namespace game2d