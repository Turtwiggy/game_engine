#include "system.hpp"

#include "actors/helpers.hpp"
#include "components.hpp"
#include "entt/helpers.hpp"
#include "lifecycle/components.hpp"
#include "maths/maths.hpp"
#include "modules/combat_wants_to_shoot/components.hpp"
#include "modules/system_particles/components.hpp"
#include "physics/components.hpp"
#include "renderer/transform.hpp"

#include "imgui.h"

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
  const auto remove_dead_parents = [&r, &dead](const entt::entity e, const entt::entity parent) -> bool {
    if (parent == entt::null || !r.valid(parent)) {
      r.emplace_or_replace<WaitForInitComponent>(e);
      dead.dead.emplace(e);
      return true;
    }
    return false;
  };

  const auto& non_physics_view =
    r.view<const DynamicTargetComponent, const SetPositionAtDynamicTarget>(entt::exclude<PhysicsBodyComponent>);
  for (const auto& [e, target_c, req_c] : non_physics_view.each()) {
    if (remove_dead_parents(e, target_c.target))
      return;
    const auto pos = get_position(r, target_c.target) + req_c.offset;
    set_position(r, e, pos);
  }

  const auto& rotation_view = r.view<const DynamicTargetComponent, const SetRotationAsDynamicTarget, TransformComponent>(
    entt::exclude<PhysicsBodyComponent>);
  for (const auto& [e, target_c, req_c, t_c] : rotation_view.each()) {
    if (remove_dead_parents(e, target_c.target))
      return;

    // set the angle of the helmet
    const float angle = r.get<TransformComponent>(target_c.target).rotation_radians.z;
    t_c.rotation_radians.z = angle;
    ImGui::Text("angle: %f", angle);

    // Set position as the offset due to rotation.
    // angle: 0. dir: {1, 0}
    // angle: PI/2(down). dir: {0, 1}
    // angle: PI(left). dir: {-1, 0}
    // angle: 3*PI/2(up). dir: {0, -1};

    // helmet: angle 0. helmet should go up.
    // helmet: angle PI/2 helmet so go right.
    // helmet: angle PI. helmet to go down
    // helmet: angle 3*PI/2. helmet to go left.

    const auto dir = engine::angle_radians_to_direction(angle - engine::HALF_PI);
    ImGui::Text("dir %f %f", dir.x, dir.y);

    const auto parent_pos = get_position(r, target_c.target);
    const auto offset = dir * 8.0f;
    ImGui::Text("offset %f %f", offset.x, offset.y);

    const auto pos = parent_pos + offset;
    set_position(r, e, pos);
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