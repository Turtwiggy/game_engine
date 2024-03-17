#include "system.hpp"

#include "audio/components.hpp"
#include "audio/helpers.hpp"
#include "components.hpp"
#include "entt/helpers.hpp"
#include "helpers/line.hpp"
#include "maths/maths.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/combat_attack_cooldown/components.hpp"
#include "modules/combat_damage/components.hpp"
#include "modules/combat_wants_to_shoot/components.hpp"
#include "modules/lerp_to_target/components.hpp"
#include "modules/lifecycle/components.hpp"
#include "modules/ux_hoverable/components.hpp"
#include "physics/components.hpp"

#include "imgui.h"

namespace game2d {

void
update_weapon_shotgun_system(entt::registry& r, const uint64_t milliseconds_dt)
{
  const float dt = milliseconds_dt / 1000.0f;
  auto& dead = get_first_component<SINGLETON_EntityBinComponent>(r);

  ImGui::Begin("Debug Shotgun");

  const auto& view =
    r.view<ShotgunComponent, HasParentComponent, AttackCooldownComponent, AABB, SpriteComponent, TransformComponent>(
      entt::exclude<WaitForInitComponent>);

  for (const auto& [entity, shotgun, parent, cooldown, aabb, sprite, shotgun_transform] : view.each()) {

    const auto& p = parent.parent;
    if (p == entt::null || !r.valid(p)) {
      dead.dead.emplace(entity); // kill this parentless entity (soz)
      continue;
    }

    const auto& [parent_aabb, parent_weapon] = r.get<const AABB, HasWeaponComponent>(p);

    // Set default position this would want to be
    // pos.position = player_aabb.center;
    // WARNING: hard-set the shotguns position as the player position
    aabb.center = parent_aabb.center;
    shotgun_transform.rotation_radians.z = 0.0f;

    // Is this unit currently selected?
    // const auto* selected = r.try_get<SelectedComponent>(p);

    // Get Target: Either an Entity or a Location
    const auto* dynamic_tgt = r.try_get<DynamicTargetComponent>(p);
    const auto* static_tgt = r.try_get<StaticTargetComponent>(p);

    // if no valid target, check line of sight
    if (parent_weapon.line_of_sight != entt::null) {
      if (dynamic_tgt == nullptr && static_tgt == nullptr) {
        dead.dead.emplace(parent_weapon.line_of_sight);
        parent_weapon.line_of_sight = entt::null;
      }
    }

    if (dynamic_tgt == nullptr && static_tgt == nullptr)
      continue;
    // Invalid dynamic target; must have a static target
    if (dynamic_tgt && !r.valid(dynamic_tgt->target)) {
      r.remove<DynamicTargetComponent>(p);
      if (!static_tgt || !static_tgt->target.has_value()) {
        continue; // If no static target either; skip all together
      }
    }

    // Assume a target by here
    glm::ivec2 target_position{ 0, 0 };
    if (dynamic_tgt != nullptr)
      target_position = r.get<AABB>(dynamic_tgt->target).center;
    if (static_tgt != nullptr)
      target_position = static_tgt->target.value();

    // dir from shotgun to target
    const auto dir_i = glm::ivec2(shotgun_transform.position.x, shotgun_transform.position.y) - target_position;
    const glm::vec2 raw_dir{ dir_i.x, dir_i.y };
    const glm::vec2 nrm_dir = engine::normalize_safe(raw_dir);

    // update "line of sight".
    // probably should not be on the "weapon" system.
    auto& los_e = parent_weapon.line_of_sight;
    if (los_e != entt::null) {
      const auto line_info = generate_line(parent_aabb.center, target_position, 10);
      auto& line_transform = r.get<TransformComponent>(los_e);
      set_transform_with_line(line_transform, line_info);
      auto& line_aabb = r.get<AABB>(los_e);
      line_aabb.center = line_transform.position;
      line_aabb.size = { line_transform.scale.x, line_transform.scale.y };
    }

    // BUGFIX: if the distance from the target to the shotgun is too close, the shotgun bugs out
    const float d2 = glm::length(raw_dir);
    ImGui::Text("D2: %f", d2);

    if (d2 >= parent_weapon.offset) {
      // simulate "picking up the gun"
      const glm::ivec2 offset = { -nrm_dir.x * parent_weapon.offset, -nrm_dir.y * parent_weapon.offset };
      aabb.center += offset;
    }

    // Rotate the gun axis to the target
    const float angle = engine::dir_to_angle_radians(nrm_dir);
    shotgun_transform.rotation_radians.z = angle;

    // Is this weapon on the left or right of the player?
    const int left_or_right = (shotgun_transform.position.x - parent_aabb.center.x);
    if (left_or_right < 0) // left
      shotgun_transform.scale.y = -glm::abs(shotgun_transform.scale.y);
    else
      shotgun_transform.scale.y = glm::abs(shotgun_transform.scale.y);

    // check if input was pressed
    bool shoot_pressed = false;
    if (const auto* shoot_request = r.try_get<WantsToShoot>(p)) {
      shoot_pressed = true;
      r.remove<WantsToShoot>(p);
    }

    // check if input was released
    // bool shoot_released = false;
    // if (const auto* release_request = r.try_get<WantsToReleaseShot>(p)) {
    //   shoot_released = true;
    //   r.remove<WantsToReleaseShot>(p);
    // }

    // fire shots in a spread
    //
    // const bool aiming = glm::abs(input.rx) > 0.3f || glm::abs(input.ry) > 0.3f;
    bool allowed_to_shoot = !cooldown.on_cooldown;
    allowed_to_shoot &= shoot_pressed; // input
    // allowed_to_shoot &= aiming;        // must be aiming

    if (allowed_to_shoot) {
      // play some audio
      // TODO: remove this.
      // This spams the audio system.
      r.emplace<AudioRequestPlayEvent>(r.create(), "SHOTGUN_SHOOT_01");

      // put gun on cooldown
      cooldown.on_cooldown = true;
      cooldown.time_between_attack_left = cooldown.time_between_attack;

      // spread the bullets out in an arc
      // const glm::vec2 r_nrm_dir = { input.rx, input.ry };
      const float angle_radians = atan2(-nrm_dir.y, -nrm_dir.x) + engine::HALF_PI;
      const float bullet_angle_degrees = 5.0f;
      const float bullet_angle_radians = engine::deg2rad(bullet_angle_degrees);

      const auto& gun_info = r.get<WeaponBulletTypeToSpawnComponent>(entity);
      for (int i = 0; i < 3; i++) {

        float angle_to_fire_at = angle_radians;
        //
        // adjust bullets in a spread
        if (i == 0) // left bullet
          angle_to_fire_at += bullet_angle_radians;
        if (i == 1) // right bullet
          angle_to_fire_at -= bullet_angle_radians;
        // i = 2 is handled as the forward bullet

        const auto new_dir = engine::angle_radians_to_direction(angle_to_fire_at);

        const auto req = create_gameplay(r, gun_info.bullet_type);
        r.get<TransformComponent>(req).position = shotgun_transform.position;
        r.get_or_emplace<HasParentComponent>(req).parent = p;

        auto& bullet_aabb = r.get<AABB>(req);
        bullet_aabb.center = { shotgun_transform.position.x, shotgun_transform.position.y };
        auto& bullet_transform = r.get<TransformComponent>(req);
        bullet_transform.rotation_radians.z = shotgun_transform.rotation_radians.z;

        auto& bullet_vel = r.get<VelocityComponent>(req);
        bullet_vel.x = new_dir.x * gun_info.bullet_speed;
        bullet_vel.y = new_dir.y * gun_info.bullet_speed;

        // Turn the bullet Live!
        r.emplace_or_replace<AttackComponent>(req, int(gun_info.bullet_damage));
        r.emplace_or_replace<EntityTimedLifecycle>(req);
      }
    }
  }

  ImGui::End();
};

} // namespace game2d