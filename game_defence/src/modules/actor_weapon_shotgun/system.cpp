#include "system.hpp"

#include "audio/components.hpp"
#include "audio/helpers.hpp"
#include "components.hpp"
#include "entt/helpers.hpp"
#include "maths/maths.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/combat_attack_cooldown/components.hpp"
#include "modules/combat_damage/components.hpp"
#include "modules/combat_wants_to_shoot/components.hpp"
#include "modules/lerp_to_target/components.hpp"
#include "modules/lifecycle/components.hpp"
#include "modules/ux_hoverable/components.hpp"
#include "physics/components.hpp"

namespace game2d {

void
update_weapon_shotgun_system(entt::registry& r, const uint64_t milliseconds_dt)
{
  const float dt = milliseconds_dt / 1000.0f;
  auto& dead = get_first_component<SINGLETON_EntityBinComponent>(r);

  const auto& view = r.view<ShotgunComponent,
                            HasParentComponent,
                            AttackCooldownComponent,
                            HasTargetPositionComponent,
                            SpriteComponent,
                            TransformComponent>(entt::exclude<WaitForInitComponent>);

  for (const auto& [entity, shotgun, parent, cooldown, pos, sprite, shotgun_transform] : view.each()) {

    const auto& p = parent.parent;
    if (p == entt::null || !r.valid(p)) {
      dead.dead.emplace(entity); // kill this parentless entity (soz)
      continue;
    }
    const auto& [player, input, player_aabb] = r.get<const PlayerComponent, const InputComponent, const AABB>(p);

    // Set default position this would want to be
    pos.position = player_aabb.center;
    shotgun_transform.rotation_radians.z = 0.0f;

    // Is this unit currently selected?
    const auto* selected = r.try_get<SelectedComponent>(p);

    // Get Target
    const auto* tgt = r.try_get<DynamicTargetComponent>(p);
    if (tgt == nullptr)
      continue;
    if (!r.valid(tgt->target)) {
      r.remove<DynamicTargetComponent>(p);
      continue;
    }

    // dir from shotgun to trget
    const auto target_aabb = r.get<AABB>(tgt->target);
    const auto dir_i = glm::ivec2(shotgun_transform.position.x, shotgun_transform.position.y) - target_aabb.center;
    glm::vec2 dir{ dir_i.x, dir_i.y };
    if (dir.x != 0.0f || dir.y != 0.0f)
      dir = glm::normalize(dir);

    // simulate "picking up the gun"
    const glm::ivec2 offset = { -dir.x * player.weapon_offset, -dir.y * player.weapon_offset };
    pos.position += offset;

    // Rotate the gun axis to the target
    const float angle = engine::dir_to_angle_radians(dir);
    shotgun_transform.rotation_radians.z = angle;

    // Is this weapon on the left or right of the player?
    const float left_or_right = (shotgun_transform.position.x - player_aabb.center.x);
    if (left_or_right < 0.0f) // left
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
    bool shoot_released = false;
    if (const auto* release_request = r.try_get<WantsToReleaseShot>(p)) {
      shoot_released = true;
      r.remove<WantsToReleaseShot>(p);
    }

    // fire shots in a spread
    //
    // const bool aiming = glm::abs(input.rx) > 0.3f || glm::abs(input.ry) > 0.3f;
    bool allowed_to_shoot = !cooldown.on_cooldown;
    allowed_to_shoot &= shoot_pressed; // input
    // allowed_to_shoot &= aiming;        // must be aiming

    if (allowed_to_shoot) {
      // play some audio
      // r.emplace<AudioRequestPlayEvent>(r.create(), "SHOTGUN_RELOAD_01");
      r.emplace<AudioRequestPlayEvent>(r.create(), "SHOTGUN_SHOOT_01");

      // put gun on cooldown
      cooldown.on_cooldown = true;
      cooldown.time_between_attack_left = cooldown.time_between_attack;

      // spread the bullets out in an arc
      // const glm::vec2 r_nrm_dir = { input.rx, input.ry };
      const float angle_radians = atan2(-dir.y, -dir.x) + engine::HALF_PI;
      const float bullet_angle_degrees = 5.0f;
      const float bullet_angle_radians = engine::deg2rad(bullet_angle_degrees);

      for (int i = 0; i < 3; i++) {
        float angle_to_fire_at = angle_radians;
        //
        // adjust bullets in a spread
        if (i == 0) // left bullet
          angle_to_fire_at += bullet_angle_radians;
        if (i == 1) // right bullet
          angle_to_fire_at -= bullet_angle_radians;

        const auto new_dir = engine::angle_radians_to_direction(angle_to_fire_at);

        const auto req = create_gameplay(r, EntityType::bullet_default);
        r.get<TransformComponent>(req).position = shotgun_transform.position;
        r.get_or_emplace<HasParentComponent>(req).parent = entity;

        auto& bullet_aabb = r.get<AABB>(req);
        bullet_aabb.center = { shotgun_transform.position.x, shotgun_transform.position.y };
        auto& bullet_transform = r.get<TransformComponent>(req);
        bullet_transform.rotation_radians.z = shotgun_transform.rotation_radians.z;

        // Turn the bullet Live!
        r.emplace_or_replace<AttackComponent>(req, 3);
        r.emplace_or_replace<EntityTimedLifecycle>(req);

        auto& bullet_vel = r.get<VelocityComponent>(req);
        bullet_vel.x = new_dir.x * shotgun.bullet_speed;
        bullet_vel.y = new_dir.y * shotgun.bullet_speed;
      }
    }
  }
};

} // namespace game2d