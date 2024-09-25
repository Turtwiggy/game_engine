#include "gun_follow_player_system.hpp"

#include "actors/actor_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/maths/maths.hpp"
#include "engine/renderer/transform.hpp"
#include "gun_follow_player_components.hpp"
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

  const auto& view = r.view<WeaponComponent, HasParentComponent, TransformComponent>();
  for (const auto [shotgun_e, weapon_c, parent_c, weapon_t] : view.each()) {
    //

    const auto& p = parent_c.parent;
    if (p == entt::null || !r.valid(p)) {
      dead.dead.emplace(shotgun_e); // kill this parentless entity (soz)
      continue;
    }

    const auto parent_pos = get_position(r, p);

    // Get the position this gun is aiming
    std::optional<glm::vec2> target_position_opt = get_parents_target(r, p);
    if (target_position_opt == std::nullopt)
      target_position_opt = mouse_pos;
    const auto target_position = target_position_opt.value();

    // Add an offset due to recoil.
    const glm::vec2 raw_dir = target_position - parent_pos;
    const glm::vec2 nrm_dir = engine::normalize_safe(raw_dir);
    auto offset = glm::vec2{ nrm_dir.x * weapon_c.offset_amount, nrm_dir.y * weapon_c.offset_amount };
    auto offset_due_to_recoil = glm::vec2{ 0.0f, 0.0f };
    weapon_c.recoil_amount -= dt * weapon_c.recoil_regain_speed;
    weapon_c.recoil_amount = glm::max(weapon_c.recoil_amount, 0.0f); // clamp above 0
    if (weapon_c.recoil_amount > 0.0f)
      offset_due_to_recoil = glm::vec2{ -nrm_dir.x * weapon_c.recoil_amount, -nrm_dir.y * weapon_c.recoil_amount };

    // set gun position
    const auto offset_pos = parent_pos + offset + offset_due_to_recoil;
    set_position(r, shotgun_e, offset_pos);

    // Rotate the gun axis to the target
    weapon_t.rotation_radians.z = engine::dir_to_angle_radians(nrm_dir) + engine::PI;
  }
}

} // namespace game2d