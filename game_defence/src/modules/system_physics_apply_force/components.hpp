#pragma once

#include <entt/entt.hpp>

namespace game2d {

// combine with: DynamicTargetComponent
struct ApplyForceToDynamicTarget
{
  float speed = 100.0f;

  // orbit tries to avoid direct collision by
  // adding a perpendicular force the closer you get
  bool orbit = true;

  // this cuts thrusters when within a distance threshold
  // to stop maximum force impact
  bool reduce_thrusters = true;
  float distance_to_reduce_thrust = 400;
};

struct ApplyForceToStaticTarget
{
  bool placeholder = true;
};

} // namespace game2d