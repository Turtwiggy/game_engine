#include "gun_follow_player_system.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/maths/maths.hpp"
#include "engine/renderer/transform.hpp"
#include "gun_follow_player_components.hpp"
#include "modules/system_hardpoint_arcs/hulls_components.hpp"
#include "modules/system_move_to_target_via_lerp/components.hpp"

namespace game2d {

std::optional<glm::vec2>
get_parents_target(entt::registry& r, const entt::entity p)
{
  // Get Target: Either an Entity or a Location
  const auto* dynamic_tgt = r.try_get<DynamicTargetComponent>(p);
  const auto* static_tgt = r.try_get<GunStaticTargetComponent>(p);

  if (dynamic_tgt == nullptr && static_tgt == nullptr)
    return std::nullopt;

  // Invalid dynamic target; must have a static target
  if (dynamic_tgt && !r.valid(dynamic_tgt->target))
    r.remove<DynamicTargetComponent>(p);

  // If no static target either; skip all together
  if (!dynamic_tgt && !static_tgt)
    return std::nullopt;

  if (dynamic_tgt != nullptr)
    return get_position(r, dynamic_tgt->target);

  if (static_tgt != nullptr)
    return static_tgt->target;

  return std::nullopt;
};

void
update_gun_follow_player_system(entt::registry& r, const glm::vec2 mouse_pos, const float dt)
{
  auto& dead = get_first_component<SINGLE_EntityBinComponent>(r);

  // static glm::vec2 hardpoint_offset{ 22.5f, 0.0f }; // rhib
  static glm::vec2 hardpoint_offset{ 5.0f, -5.0f }; // dinghy
  // imgui_draw_vec2("hardpoint_offset", hardpoint_offset);

  const auto& view = r.view<WeaponComponent, HasParentComponent, TransformComponent, const HardpointComponent>();
  for (const auto [shotgun_e, weapon_c, parent_c, weapon_t, hardpoint_c] : view.each()) {

    const auto p = parent_c.parent;
    if (!r.valid(p) || p == entt::null) {
      r.remove<HasParentComponent>(shotgun_e);
      continue;
    }

    // gunpoint base
    const auto tl_offset = glm::vec2{ hardpoint_c.data.x_rel_tl, hardpoint_c.data.y_rel_tl };
    const auto& t_c = r.get<TransformComponent>(p);
    const float fwd = t_c.rotation_radians.z;
    const auto pos = glm::vec2{ t_c.position.x, t_c.position.y };
    const auto size = glm::vec2{ t_c.scale.x, t_c.scale.y };
    const auto tl = pos - (0.5f * size);
    const auto rel_tl = (tl - pos) + tl_offset;
    const auto rotated_point = engine::rotate_point(rel_tl, fwd);
    const auto hardpoint_pos = pos + rotated_point;

    // Add an offset due to recoil.
    // auto offset_due_to_recoil = glm::vec2{ 0.0f, 0.0f };
    // weapon_c.recoil_amount -= dt * weapon_c.recoil_regain_speed;
    // weapon_c.recoil_amount = glm::max(weapon_c.recoil_amount, 0.0f); // clamp above 0
    // if (weapon_c.recoil_amount > 0.0f)
    //   offset_due_to_recoil = glm::vec2{ -nrm_dir.x * weapon_c.recoil_amount, -nrm_dir.y * weapon_c.recoil_amount };

    // set gun position
    set_position(r, shotgun_e, hardpoint_pos);
  }
}

} // namespace game2d