#include "system.hpp"

#include "actors/actors.hpp"
#include "actors/bags/bullets.hpp"
#include "actors/helpers.hpp"
#include "audio/components.hpp"
#include "components.hpp"
#include "entt/helpers.hpp"
#include "lifecycle/components.hpp"
#include "magic_enum.hpp"
#include "maths/maths.hpp"
#include "modules/combat_attack_cooldown/components.hpp"
#include "modules/combat_damage/components.hpp"
#include "modules/combat_wants_to_shoot/components.hpp"
#include "renderer/transform.hpp"

#include <fmt/core.h>

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
update_weapon_shotgun_system(entt::registry& r, const uint64_t milliseconds_dt)
{
  auto& dead = get_first_component<SINGLETON_EntityBinComponent>(r);
  const float dt = milliseconds_dt / 1000.0f;

  const auto& view = r.view<ShotgunComponent, HasParentComponent, TransformComponent>(entt::exclude<WaitForInitComponent>);

  for (const auto& [shotgun_e, shotgun_c, parent, shotgun_t] : view.each()) {

    const auto& p = parent.parent;
    if (p == entt::null || !r.valid(p)) {
      dead.dead.emplace(shotgun_e); // kill this parentless entity (soz)
      continue;
    }

    // Get the position this gun is aiming
    const auto target_position_opt = get_parents_target(r, p);
    if (target_position_opt == std::nullopt)
      continue;
    const auto target_position = target_position_opt.value();

    // distance from parent
    const auto parent_pos = get_position(r, p);
    const glm::vec2 raw_dir = target_position - parent_pos;
    const glm::vec2 nrm_dir = engine::normalize_safe(raw_dir);

    // Add an offset.
    auto offset = glm::vec2{ nrm_dir.x * shotgun_c.offset_amount, nrm_dir.y * shotgun_c.offset_amount };

    // Add an offset due to recoil.
    auto offset_due_to_recoil = glm::vec2{ 0.0f, 0.0f };
    shotgun_c.recoil_amount -= dt * shotgun_c.recoil_regain_speed;
    shotgun_c.recoil_amount = glm::max(shotgun_c.recoil_amount, 0.0f); // clamp above 0
    if (shotgun_c.recoil_amount > 0.0f)
      offset_due_to_recoil = glm::vec2{ -nrm_dir.x * shotgun_c.recoil_amount, -nrm_dir.y * shotgun_c.recoil_amount };

    // Set gun position
    {
      const auto offset_pos = parent_pos + offset + offset_due_to_recoil;
      set_position(r, shotgun_e, offset_pos);
    }

    // recalculate
    // raw_dir = offset_pos - target_position;
    // nrm_dir = engine::normalize_safe(raw_dir);

    // Rotate the gun axis to the target
    shotgun_t.rotation_radians.z = engine::dir_to_angle_radians(nrm_dir) + engine::PI;

    // check if input was pressed
    bool shoot_pressed = false;
    if (const auto* shoot_request = r.try_get<WantsToShoot>(p)) {
      shoot_pressed = true;
      r.remove<WantsToShoot>(p);
    }

    // check if input was released
    bool shoot_released = false;
    if (const auto* release_request = r.try_get<WantsToReleaseShot>(p)) {
      shoot_released = true;
      r.remove<WantsToReleaseShot>(p);
    }

    if (shoot_pressed && !r.try_get<AbleToShoot>(shotgun_e)) {
      // not able to shoot...

      // spawn particles "pfft"; you couldnt shoot
      const float pfft_speed = 0.1f;

      DataParticle desc;
      desc.time_to_live_ms = 1000;
      desc.pos = parent_pos + offset + offset_due_to_recoil;
      desc.velocity = { -nrm_dir.x * pfft_speed, -glm::abs(nrm_dir.y * pfft_speed) };
      desc.start_size = 10;
      desc.end_size = 0;
      desc.sprite = "EMPTY";
      const auto e = Factory_DataParticle::create(r, desc);

      continue;
    }

    // Gun may not have a cooldown component
    auto* cooldown = r.try_get<AttackCooldownComponent>(p);

    // fire shots in a spread
    // const bool aiming = glm::abs(input.rx) > 0.3f || glm::abs(input.ry) > 0.3f;
    bool allowed_to_shoot = shoot_pressed;
    if (cooldown)
      allowed_to_shoot &= !cooldown->on_cooldown;

    if (allowed_to_shoot) {
      // TODO: improve this. This spams the audio system.
      create_empty<AudioRequestPlayEvent>(r, AudioRequestPlayEvent{ "SHOTGUN_SHOOT_01" });

      // Add knockback to the shotgun
      shotgun_c.recoil_amount = shotgun_c.recoil_amount_max;

      // put gun on cooldown
      if (cooldown) {
        cooldown->on_cooldown = true;
        cooldown->time_between_attack_left = cooldown->time_between_attack;
      }

      // spread the bullets out in an arc
      // const glm::vec2 r_nrm_dir = { input.rx, input.ry };
      // const float angle_radians = engine::dir_to_angle_radians(nrm_dir);
      // constexpr float bullet_angle_degrees = 5.0f;
      // constexpr float bullet_angle_radians = bullet_angle_degrees * engine::Deg2Rad;

      // give all the bullets the same attack id

      for (int i = 2; i < 3; i++) {

        // float angle_to_fire_at = angle_radians;
        // if (i == 0) // left bullet
        //   angle_to_fire_at += bullet_angle_radians;
        // if (i == 1) // right bullet
        //   angle_to_fire_at -= bullet_angle_radians;
        // i = 2 is handled as the forward bullet

        // fmt::println("firing gun at angle(degrees): " << angle_to_fire_at * engine::Rad2Deg << std::endl;
        // const auto new_dir = engine::angle_radians_to_direction(angle_to_fire_at);

        const glm::vec2 bullet_position = parent_pos + (offset);

        DataBullet desc(shotgun_c.bullet_type);
        desc.pos = bullet_position;
        desc.dir = nrm_dir;
        desc.rotation = shotgun_t.rotation_radians.z;
        desc.team = r.get<TeamComponent>(p).team;
        desc.parent = p;
        desc.type = shotgun_c.bullet_type;
        const auto bullet_e = Factory_DataBullet::create(r, desc);

        //
      }
    }
  }
};

} // namespace game2d