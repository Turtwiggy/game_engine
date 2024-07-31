#include "system.hpp"

#include "components.hpp"
#include "glm/glm.hpp"
#include "lifecycle/components.hpp"
#include "maths/maths.hpp"
#include "modules/actors/helpers.hpp"
#include "physics/components.hpp"
#include "renderer/transform.hpp"

namespace game2d {

void
update_set_velocity_to_target_system(entt::registry& r, const float& dt)
{
  // If follow parent...
  // Set your target position as your parents target position.
  //
  const auto& follow_view = r.view<FollowTargetComponent, HasTargetPositionComponent>();
  for (const auto& [e, follow, target] : follow_view.each()) {
    target.position = r.get<AABB>(follow.target).center;
  }

  // set velocity to target
  {
    const auto& view = r.view<VelocityComponent, const HasTargetPositionComponent, const SetVelocityToTargetComponent>(
      entt::exclude<WaitForInitComponent>);
    for (const auto& [e, vel, target, lerp] : view.each()) {
      if (!target.position.has_value())
        continue;
      const glm::ivec2 a = get_position(r, e);
      const glm::ivec2 b = target.position.value();
      const glm::vec2 nrm_dir = engine::normalize_safe(b - a);
      vel.x = nrm_dir.x * vel.base_speed;
      vel.y = nrm_dir.y * vel.base_speed;
    }
  }

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

  // https://www.youtube.com/watch?v=LSNQuFEDOyQ
  const float decay = 16;
  const auto exp_decay = [](float a, float b, float decay, float dt) -> float {
    return b + (a - b) * glm::exp(-decay * dt);
  };
  // e.g. const float a = exp_decay(a, b, decay, dt);

  const auto& view = r.view<AABB, const HasTargetPositionComponent, LerpingToTarget>(entt::exclude<WaitForInitComponent>);
  for (const auto& [e, aabb, target, lerp_info] : view.each()) {
    if (!target.position.has_value())
      continue;
    // Lerp from current postion to target position

    // const glm::ivec2 a = { aabb.center.x, aabb.center.y };
    // const glm::ivec2 b = target.position.value();
    // const glm::vec2 nrm_dir = engine::normalize_safe(b - a);
    // vel.x = nrm_dir.x * vel.base_speed;
    // vel.y = nrm_dir.y * vel.base_speed;

    // const int pos_x = static_cast<int>(exp_decay(a.x, b.x, 32.0f, dt));
    // const int pos_y = static_cast<int>(exp_decay(a.y, b.y, 32.0f, dt));
    // if (a.x == pos_x && a.y == pos_y) // lerp not lerping as we're too close...
    //   aabb.center = { b.x, b.y };
    // else
    //   aabb.center = { pos_x, pos_y };

    // clamp to max position
    if (lerp_info.t >= 1.0f)
      lerp_info.t = 1.0f;

    const auto a = lerp_info.a;
    const auto b = lerp_info.b;

    const float decay = 20; // higher number = faster to destination
    const int pos_x = static_cast<int>(exp_decay(a.x, b.x, decay, lerp_info.t));
    const int pos_y = static_cast<int>(exp_decay(a.y, b.y, decay, lerp_info.t));
    aabb.center = { pos_x, pos_y };

    lerp_info.t += dt;

    // check not out of bounds
    if (lerp_info.t >= 1.0f)
      r.remove<LerpingToTarget>(e);
  }
};

} // namespace game2d