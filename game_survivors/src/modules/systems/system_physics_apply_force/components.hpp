#pragma once

#include <entt/fwd.hpp>
#include <glm/fwd.hpp>

namespace game2d {

struct PhysicsDynamicTarget
{
  entt::entity target = entt::null;
};

struct ApplyForceInDirectionComponent
{
  glm::vec2 tgt_vel{ 0.0f, 0.0f };
};

// combine with: DynamicTargetComponent
struct ApplyForceToDynamicTarget
{
  // orbit tries to avoid direct collision by
  // adding a perpendicular force the closer you get
  bool orbit = true;

  // this cuts thrusters when within a distance threshold
  // to stop maximum force impact
  bool reduce_thrusters = true;
  float distance_to_reduce_thrust = 6;
};

struct ApplyLinearVelocityToDynamicTarget
{
  bool placeholder = true;
};

struct ApplyForceToApproachTargetFromAngle
{
  float angle_rad = 0.0f;
};

} // namespace game2d