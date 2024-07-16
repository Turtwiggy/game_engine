#include "system.hpp"

#include "audio/components.hpp"
#include "components.hpp"
#include "entt/helpers.hpp"
#include "lifecycle/components.hpp"
#include "maths/maths.hpp"
#include "modules/combat_attack_cooldown/components.hpp"
#include "modules/combat_damage/components.hpp"
#include "modules/combat_wants_to_shoot/components.hpp"
#include "modules/lerp_to_target/components.hpp"
#include "physics/components.hpp"
#include "renderer/transform.hpp"

namespace game2d {

std::optional<glm::ivec2>
get_parent_target_position(entt::registry& r, const entt::entity& p)
{
  // Get Target: Either an Entity or a Location
  const auto* dynamic_tgt = r.try_get<DynamicTargetComponent>(p);
  const auto* static_tgt = r.try_get<StaticTargetComponent>(p);

  if (dynamic_tgt == nullptr && static_tgt == nullptr)
    return std::nullopt;

  // Invalid dynamic target; must have a static target
  if (dynamic_tgt && !r.valid(dynamic_tgt->target))
    r.remove<DynamicTargetComponent>(p);

  // If no static target either; skip all together
  if (!dynamic_tgt && (!static_tgt || !static_tgt->target.has_value()))
    return std::nullopt;

  if (dynamic_tgt != nullptr)
    return r.get<AABB>(dynamic_tgt->target).center;

  if (static_tgt != nullptr)
    return static_tgt->target.value();

  return std::nullopt;
};

void
update_weapon_shotgun_system(entt::registry& r, const uint64_t milliseconds_dt)
{
  auto& dead = get_first_component<SINGLETON_EntityBinComponent>(r);
  const float dt = milliseconds_dt / 1000.0f;

  const auto& view = r.view<ShotgunComponent,
                            HasParentComponent,
                            AABB,
                            TransformComponent,
                            // const HasTargetPositionComponent,
                            VelocityComponent>(entt::exclude<WaitForInitComponent>);

  for (const auto& [entity, shotgun, parent, gun_aabb, shotgun_transform, gun_velocity] : view.each()) {

    const auto& p = parent.parent;
    if (p == entt::null || !r.valid(p)) {
      dead.dead.emplace(entity); // kill this parentless entity (soz)
      continue;
    }

    const auto& [parent_aabb, parent_team] = r.get<const AABB, const TeamComponent>(p);

    // Get the position this gun is aiming
    const auto target_position_opt = get_parent_target_position(r, p);
    if (target_position_opt == std::nullopt)
      continue;
    const glm::ivec2 target_position = target_position_opt.value();

    // set gun position
    gun_aabb.center = glm::ivec2(parent_aabb.center);

    // distance from parent
    auto dir_i = gun_aabb.center - target_position;
    auto raw_dir = glm::vec2{ dir_i.x, dir_i.y };
    auto nrm_dir = engine::normalize_safe(raw_dir);

    // Add an offset.
    auto offset = glm::vec2{ -nrm_dir.x * shotgun.offset_amount, -nrm_dir.y * shotgun.offset_amount };

    // Add an offset due to recoil.
    shotgun.recoil_amount -= dt * shotgun.recoil_regain_speed;
    shotgun.recoil_amount = glm::max(shotgun.recoil_amount, 0.0f); // clamp above 0
    if (shotgun.recoil_amount > 0.0f)
      offset += glm::vec2{ nrm_dir.x * shotgun.recoil_amount, nrm_dir.y * shotgun.recoil_amount };

    gun_aabb.center += offset;

    // recalculate
    //
    dir_i = gun_aabb.center - target_position;
    raw_dir = glm::vec2{ dir_i.x, dir_i.y };
    nrm_dir = engine::normalize_safe(raw_dir);

    // Rotate the gun axis to the target
    const float angle = engine::dir_to_angle_radians(nrm_dir);
    shotgun_transform.rotation_radians.z = angle;

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
      shotgun.recoil_amount = shotgun.recoil_amount_max;

      // put gun on cooldown
      if (cooldown) {
        cooldown->on_cooldown = true;
        cooldown->time_between_attack_left = cooldown->time_between_attack;
      }

      // spread the bullets out in an arc
      // const glm::vec2 r_nrm_dir = { input.rx, input.ry };
      const float angle_radians = engine::dir_to_angle_radians(nrm_dir);
      constexpr float bullet_angle_degrees = 5.0f;
      constexpr float bullet_angle_radians = bullet_angle_degrees * engine::Deg2Rad;

      const auto& bullet_info = r.get<WeaponBulletTypeToSpawnComponent>(entity);
      for (int i = 0; i < shotgun.bullets_to_spawn; i++) {

        float angle_to_fire_at = angle_radians;
        if (i == 0) // left bullet
          angle_to_fire_at += bullet_angle_radians;
        if (i == 1) // right bullet
          angle_to_fire_at -= bullet_angle_radians;
        // i = 2 is handled as the forward bullet
        // fmt::println("firing gun at angle(degrees): " << angle_to_fire_at * engine::Rad2Deg << std::endl;
        const auto new_dir = engine::angle_radians_to_direction(angle_to_fire_at);

        const auto req = create_gameplay(r, bullet_info.bullet_type);
        r.get_or_emplace<HasParentComponent>(req).parent = p;

        const glm::ivec2 bullet_position = gun_aabb.center + glm::ivec2(offset.x, offset.y);

        auto& bullet_transform = r.get<TransformComponent>(req);
        bullet_transform.position = { bullet_position.x, bullet_position.y, 0.0f };
        bullet_transform.rotation_radians.z = shotgun_transform.rotation_radians.z;
        auto& bullet_aabb = r.get<AABB>(req);
        bullet_aabb.center = bullet_position;

        auto& bullet_vel = r.get<VelocityComponent>(req);
        bullet_vel.x = (new_dir.x * bullet_info.bullet_speed);
        bullet_vel.y = (new_dir.y * bullet_info.bullet_speed);

        // Turn the bullet Live!
        r.emplace_or_replace<TeamComponent>(req, parent_team.team);
        r.emplace_or_replace<AttackComponent>(req, int(bullet_info.bullet_damage));
        r.emplace_or_replace<EntityTimedLifecycle>(req);
      }
    }
  }
};

} // namespace game2d