#pragma once

#include "glm/glm.hpp"
#include <entt/entt.hpp>

namespace game2d {

//
// weapon components
//

struct FollowTargetComponent
{
  entt::entity target;
};

struct HasTargetPositionComponent
{
  glm::ivec2 position{ 0, 0 };
};

struct SetVelocityToTargetComponent
{
  bool placeholder = true;
};

} // namespace game2d