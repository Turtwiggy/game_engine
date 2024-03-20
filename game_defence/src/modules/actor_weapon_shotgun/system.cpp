#include "system.hpp"

#include "audio/components.hpp"
#include "components.hpp"
#include "entt/helpers.hpp"
#include "maths/maths.hpp"
#include "modules/combat_attack_cooldown/components.hpp"
#include "modules/combat_damage/components.hpp"
#include "modules/combat_wants_to_shoot/components.hpp"
#include "modules/lerp_to_target/components.hpp"
#include "modules/lifecycle/components.hpp"
#include "physics/components.hpp"

#include "imgui.h"

namespace game2d {

std::optional<glm::ivec2>
get_target_position(entt::registry& r, const entt::entity& e, const entt::entity& p)
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
  const float dt = milliseconds_dt / 1000.0f;
  auto& dead = get_first_component<SINGLETON_EntityBinComponent>(r);

  ImGui::Begin("Debug Shotgun");

  const auto& view =
    r.view<ShotgunComponent, HasParentComponent, AttackCooldownComponent, AABB, TransformComponent, VelocityComponent>(
      entt::exclude<WaitForInitComponent>);

  for (const auto& [entity, shotgun, parent, cooldown, gun_aabb, shotgun_transform, gun_velocity] : view.each()) {

    const auto& p = parent.parent;
    if (p == entt::null || !r.valid(p)) {
      dead.dead.emplace(entity); // kill this parentless entity (soz)
      continue;
    }

    const auto& [parent_aabb, parent_weapon] = r.get<const AABB, HasWeaponComponent>(p);

    // Move this weapon to its parent
    // move_target.position = parent_aabb.center;
    gun_aabb.center = parent_aabb.center;

    // Get the position this gun is aiming
    const auto target_position_opt = get_target_position(r, entity, p);
    if (target_position_opt == std::nullopt)
      continue;
    const glm::ivec2 target_position = target_position_opt.value();

    // dir from shotgun to target
    const auto dir_i = gun_aabb.center - target_position;
    const glm::vec2 raw_dir{ dir_i.x, dir_i.y };
    const glm::vec2 nrm_dir = engine::normalize_safe(raw_dir);

    // BUGFIX: if the distance from the target to the shotgun is too close, the shotgun bugs out
    // const float d = glm::length(raw_dir);
    // // simulate "picking up the gun"
    // if (d > 20 && (glm::abs(nrm_dir.x) > 0 || glm::abs(nrm_dir.y) > 0)) {
    const glm::ivec2 offset = { -nrm_dir.x * parent_weapon.offset, -nrm_dir.y * parent_weapon.offset };
    gun_aabb.center += offset;
    // }

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

    // fire shots in a spread
    // const bool aiming = glm::abs(input.rx) > 0.3f || glm::abs(input.ry) > 0.3f;
    bool allowed_to_shoot = !cooldown.on_cooldown;
    allowed_to_shoot &= shoot_pressed; // input

    if (allowed_to_shoot) {
      // TODO: improve this. This spams the audio system.
      r.emplace<AudioRequestPlayEvent>(r.create(), "SHOTGUN_SHOOT_01");

      // put gun on cooldown
      cooldown.on_cooldown = true;
      cooldown.time_between_attack_left = cooldown.time_between_attack;

      // spread the bullets out in an arc
      // const glm::vec2 r_nrm_dir = { input.rx, input.ry };
      const float angle_radians = engine::dir_to_angle_radians(nrm_dir);
      const float bullet_angle_degrees = 5.0f;
      const float bullet_angle_radians = engine::deg2rad(bullet_angle_degrees);

      const auto& bullet_info = r.get<WeaponBulletTypeToSpawnComponent>(entity);
      for (int i = 0; i < shotgun.bullets_to_spawn; i++) {

        float angle_to_fire_at = angle_radians;
        if (i == 0) // left bullet
          angle_to_fire_at += bullet_angle_radians;
        if (i == 1) // right bullet
          angle_to_fire_at -= bullet_angle_radians;
        // i = 2 is handled as the forward bullet
        std::cout << "firing gun at angle(degrees): " << angle_to_fire_at * engine::Rad2Deg << std::endl;
        const auto new_dir = engine::angle_radians_to_direction(angle_to_fire_at);

        const auto req = create_gameplay(r, bullet_info.bullet_type);
        r.get<TransformComponent>(req).position = shotgun_transform.position;
        r.get_or_emplace<HasParentComponent>(req).parent = p;

        auto& bullet_aabb = r.get<AABB>(req);
        bullet_aabb.center = { shotgun_transform.position.x, shotgun_transform.position.y };
        auto& bullet_transform = r.get<TransformComponent>(req);
        bullet_transform.rotation_radians.z = shotgun_transform.rotation_radians.z;

        auto& bullet_vel = r.get<VelocityComponent>(req);
        bullet_vel.x = (new_dir.x * bullet_info.bullet_speed);
        bullet_vel.y = (new_dir.y * bullet_info.bullet_speed);

        // Turn the bullet Live!
        r.emplace_or_replace<AttackComponent>(req, int(bullet_info.bullet_damage));
        r.emplace_or_replace<EntityTimedLifecycle>(req);
      }
    }
  }

  ImGui::End();
};

} // namespace game2d