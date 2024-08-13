#include "system.hpp"

#include "components.hpp"
#include "entt/helpers.hpp"
#include "lifecycle/components.hpp"
#include "modules/actors/helpers.hpp"
#include "modules/combat_wants_to_shoot/components.hpp"
#include "modules/system_particles/components.hpp"
#include "physics/components.hpp"
#include "renderer/transform.hpp"

namespace game2d {

// https://www.youtube.com/watch?v=LSNQuFEDOyQ
const auto exp_decay = [](float a, float b, float decay, float dt) -> float {
  //
  return b + (a - b) * glm::exp(-decay * dt);
  //
};

void
update_move_to_target_via_lerp(entt::registry& r, const float& dt)
{
  //
  // Do the lerp and update position
  //
  const auto& view = r.view<LerpingToTarget>(entt::exclude<WaitForInitComponent>);
  for (const auto& [e, lerp_info] : view.each()) {
    if (lerp_info.t >= 1.0f)
      lerp_info.t = 1.0f;
    lerp_info.t += dt;

    const auto& a = lerp_info.a;
    const auto& b = lerp_info.b;

    const float decay = 20; // higher number = faster to destination
    const float pos_x = exp_decay(a.x, b.x, decay, lerp_info.t);
    const float pos_y = exp_decay(a.y, b.y, decay, lerp_info.t);

    set_position(r, e, { pos_x, pos_y });

    // check not out of bounds
    if (lerp_info.t >= 1.0f)
      r.remove<LerpingToTarget>(e);
  }

  //
  // Follow your parent. // e.g. particle systems.
  //
  auto& dead = get_first_component<SINGLETON_EntityBinComponent>(r);
  const auto& non_physics_view =
    r.view<const DynamicTargetComponent, const SetPositionAtDynamicTarget>(entt::exclude<PhysicsBodyComponent>);
  for (const auto& [e, target_c, req_c] : non_physics_view.each()) {

    if (target_c.target == entt::null || !r.valid(target_c.target)) {
      r.emplace_or_replace<WaitForInitComponent>(e);
      dead.dead.emplace(e);
      continue;
    }

    const auto target_position = get_position(r, target_c.target) + req_c.offset;
    set_position(r, e, target_position);
  }

  //
  // This moves TransformComponent that do not have AABB e.g. particles
  //
  const auto& particle_view =
    r.view<const VelocityTemporaryComponent, TransformComponent>(entt::exclude<PhysicsBodyComponent>);
  for (const auto& [e, vel, transform_c] : particle_view.each()) {
    transform_c.position.x += vel.x * dt;
    transform_c.position.y += vel.y * dt;
  }
}

} // namespace game2d