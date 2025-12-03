#include "move_to_target_via_lerp_system.hpp"

#include "components.hpp"
#include "engine/actors/actor_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/imgui/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/maths/maths.hpp"
#include "engine/physics/physics_components.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/core/sprites/sprite_helpers.hpp"
#include "modules/systems/system_particles/components.hpp"

namespace game2d {

// https://www.youtube.com/watch?v=LSNQuFEDOyQ
const auto exp_decay = [](float a, float b, float decay, float dt) -> float {
  //
  return b + (a - b) * expf(-decay * dt);
  //
};

const auto remove_dead_parents =
  [](entt::registry& r, SINGLE_EntityBinComponent& dead, const entt::entity e, const entt::entity parent) -> bool {
  if (parent == entt::null || !r.valid(parent)) {
    r.emplace_or_replace<WaitForInitComponent>(e);
    dead.dead.push_back(e);
    return true;
  }
  return false;
};

void
update_move_to_target_via_lerp(entt::registry& r, const float& dt)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif
  // update static lerps
  {
    const auto& view = r.view<LerpToFixedTarget, TransformComponent>(entt::exclude<WaitForInitComponent>);
    for (const auto& [e, info, t_c] : view.each()) {
      if (info.t >= 1.0f)
        info.t = 1.0f;
      info.t += dt;

      const float pos_x = exp_decay(info.a.x, info.b.x, info.speed, info.t);
      const float pos_y = exp_decay(info.a.y, info.b.y, info.speed, info.t);
      t_c.position = { pos_x, pos_y, 0.0f };

      // check not out of bounds
      if (info.t >= 1.0f)
        r.remove<LerpToFixedTarget>(e);
    }
  }

  // update dynamic lerps
  {
    const auto& view = r.view<LerpToMovingTarget, TransformComponent>(entt::exclude<WaitForInitComponent>);
    for (const auto& [e, info, t_c] : view.each()) {
      const float pos_x = exp_decay(info.a.x, info.b.x, info.speed, dt);
      const float pos_y = exp_decay(info.a.y, info.b.y, info.speed, dt);
      t_c.position = { pos_x, pos_y, 0.0f };
    }
  }

  // Follow your parent. // e.g. particle systems.
  auto& dead = get_first_component<SINGLE_EntityBinComponent>(r);

  const auto& non_physics_view = r.view<const DynamicTargetComponent, const SetPositionAtDynamicTarget, TransformComponent>(
    entt::exclude<PhysicsBodyComponent>);
  for (const auto& [e, target_c, req_c, t_c] : non_physics_view.each()) {
    if (remove_dead_parents(r, dead, e, target_c.target))
      continue;
    const auto pos = get_position(r, target_c.target) + req_c.offset;
    t_c.position = glm::vec3{ pos.x, pos.y, 0.0f };
  }

  //
  //
  const auto& rotation_view =
    r.view<const DynamicTargetComponent, const SetPositionAtDynamicTargetFromRotation, const TransformComponent>(
      entt::exclude<PhysicsBodyComponent>);
  for (const auto& [e, target_c, set_pos_c, emitter_t] : rotation_view.each()) {
    if (remove_dead_parents(r, dead, e, target_c.target))
      continue;
    const auto p = target_c.target;

    const auto tl_offset = set_pos_c.offset;
    const auto& p_t = r.get<TransformComponent>(p);
    const auto p_pos = glm::vec2{ p_t.position.x, p_t.position.y };
    const auto p_size = glm::vec2{ p_t.scale.x, p_t.scale.y };
    const float p_fwd = p_t.rotation_radians.z; // parents_dir
    const auto p_tl = p_pos - (0.5f * p_size);
    const auto rel_tl = (p_tl - p_pos) + tl_offset;
    const auto rotated_point = engine::rotate_point(rel_tl, p_fwd);
    const auto pos = p_pos + rotated_point;

#if defined(_DEBUG)
    // Sprite s;
    // s.sprite = "EMPTY";
    // s.pos = { pos.x, pos.y };
    // s.size = { 10, 10 };
    // s.col = { 1.0f, 0.0f, 0.0f, 1.0f };
    // s.z_idx = ZLayer::FOREGROUND;
    // draw_sprite(r, s);
#endif

    set_position(r, e, pos);
  }

  //
  // This moves TransformComponent that do not have AABB e.g. particles
  //
  const auto& particle_view = r.view<const VelocityComponent, TransformComponent>(entt::exclude<PhysicsBodyComponent>);
  for (const auto& [e, vel, transform_c] : particle_view.each()) {
    transform_c.position.x += vel.x * dt;
    transform_c.position.y += vel.y * dt;
  }

  // ImGui::Text("particles: %zu", particle_view.size_hint());
}

} // namespace game2d