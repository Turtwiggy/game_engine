#include "system.hpp"

#include "components.hpp"
#include "maths/maths.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/combat_attack_cooldown/components.hpp"
#include "modules/combat_damage/components.hpp"
#include "modules/combat_wants_to_shoot/components.hpp"
#include "modules/lerp_to_target/components.hpp"
#include "modules/lifecycle/components.hpp"
#include "physics/components.hpp"

namespace game2d {

void
update_weapon_shotgun_system(entt::registry& r, const uint64_t milliseconds_dt)
{
  const float dt = milliseconds_dt / 1000.0f;

  const auto& view = r.view<ShotgunComponent,
                            HasParentComponent,
                            AttackCooldownComponent,
                            HasTargetPositionComponent,
                            SpriteComponent,
                            TransformComponent>(entt::exclude<WaitForInitComponent>);

  for (const auto& [entity, shotgun, parent, cooldown, target, sprite, transform] : view.each()) {

    const auto& p = parent.parent;
    if (p == entt::null)
      continue;
    const auto& [player, input, player_aabb] = r.get<const PlayerComponent, const InputComponent, const AABB>(p);

    // where does this weapon want to be?
    const glm::ivec2 offset = { -input.rx * player.weapon_offset, -input.ry * player.weapon_offset };
    target.position = player_aabb.center + offset;

    // Is this weapon on the left or right of the player?
    if (transform.position.x - player_aabb.center.x < 0)
      transform.scale.x = -glm::abs(transform.scale.x);
    else
      transform.scale.x = glm::abs(transform.scale.x);

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

    //
    // fire shots in a spread
    //
    const bool aiming = glm::abs(input.rx) > 0.3f || glm::abs(input.ry) > 0.3f;
    bool allowed_to_shoot = !cooldown.on_cooldown;
    allowed_to_shoot &= aiming;        // must be aiming
    allowed_to_shoot &= shoot_pressed; // input

    if (allowed_to_shoot) {
      // put gun on cooldown
      cooldown.on_cooldown = true;
      cooldown.time_between_attack_left = cooldown.time_between_attack;

      // spread the bullets out in an arc
      const glm::vec2 r_nrm_dir = { input.rx, input.ry };
      const float angle_radians = atan2(-r_nrm_dir.y, -r_nrm_dir.x) + engine::HALF_PI;
      const float bullet_angle_degrees = 5.0f;
      const float bullet_angle_radians = engine::deg2rad(bullet_angle_degrees);

      for (int i = 0; i < 3; i++) {
        float angle_to_fire_at = angle_radians;
        // adjust bullets in a spread
        if (i == 0) // left bullet
          angle_to_fire_at += bullet_angle_radians;
        if (i == 1) // right bullet
          angle_to_fire_at -= bullet_angle_radians;

        const auto new_dir = engine::angle_radians_to_direction(angle_to_fire_at);

        const auto req = create_gameplay(r, EntityType::bullet_default);
        r.get<TransformComponent>(req).position = transform.position;
        r.get_or_emplace<HasParentComponent>(req).parent = entity;

        auto& bullet_aabb = r.get<AABB>(req);
        bullet_aabb.center = { transform.position.x, transform.position.y };
        auto& bullet_transform = r.get<TransformComponent>(req);
        bullet_transform.rotation_radians.z = transform.rotation_radians.z;

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