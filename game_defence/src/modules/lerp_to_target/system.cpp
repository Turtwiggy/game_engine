#include "system.hpp"

#include "components.hpp"
#include "lifecycle/components.hpp"
#include "maths/maths.hpp"
#include "physics/components.hpp"
#include "renderer/transform.hpp"

namespace game2d {

void
update_set_velocity_to_target_system(entt::registry& r, const float& dt)
{
  //
  // If follow parent...
  // Set your target position as your parents target position.
  //
  const auto& follow_view = r.view<FollowTargetComponent, HasTargetPositionComponent>();
  for (const auto& [e, follow, target] : follow_view.each()) {
    const auto& p_pos = r.get<AABB>(follow.target);
    target.position = p_pos.center;
  }

  //
  // set velocity to target
  //
  const auto& view =
    r.view<VelocityComponent, const AABB, const HasTargetPositionComponent, const SetVelocityToTargetComponent>(
      entt::exclude<WaitForInitComponent>);
  for (const auto& [e, vel, aabb, target, lerp] : view.each()) {
    if (!target.position.has_value())
      continue;
    const glm::ivec2 a = { aabb.center.x, aabb.center.y };
    const glm::ivec2 b = target.position.value();
    const glm::vec2 nrm_dir = engine::normalize_safe(b - a);
    vel.x = nrm_dir.x * vel.base_speed;
    vel.y = nrm_dir.y * vel.base_speed;
  }

  //
  // This moves TransformComponent
  // That do not have AABB components.
  //
  const auto& t_view = r.view<VelocityComponent, TransformComponent>(entt::exclude<WaitForInitComponent, AABB>);
  for (const auto& [e, vel, t] : t_view.each()) {

    vel.remainder_x += vel.x * dt;
    vel.remainder_y += vel.y * dt;

    const int x = static_cast<int>(vel.remainder_x);
    const int y = static_cast<int>(vel.remainder_y);

    vel.remainder_x -= x;
    vel.remainder_y -= y;

    t.position.x += x;
    t.position.y += y;
  }
};

} // namespace game2d