#pragma once

#include "modules/animation/components.hpp"
#include <entt/entt.hpp>

namespace game2d {

// combine with: DynamicTargetComponent
struct ApplyForceToDynamicTarget
{
  bool placeholder = true;
};

struct ApplyForceToStaticTarget
{
  bool placeholder = true;
};

struct OrbitEntity
{
  entt::entity e = entt::null;
  float distance = 1.0f;
  float rotate_speed = 1.0f;
  float theta = 0.0f; // 0 and 2PI

  entt::entity debug_e = entt::null;
};

} // namespace game2d