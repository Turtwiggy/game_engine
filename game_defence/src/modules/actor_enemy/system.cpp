#include "system.hpp"

#include "entt/helpers.hpp"
#include "modules/actor_enemy/components.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/combat_attack_cooldown/components.hpp"
#include "modules/combat_damage/components.hpp"
#include "modules/combat_wants_to_shoot/components.hpp"
#include "modules/lerp_to_target/components.hpp"
#include "modules/lifecycle/components.hpp"
#include "physics/components.hpp"
#include "renderer/transform.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/compatibility.hpp> // lerp

namespace game2d {

void
update_enemy_system(entt::registry& r, const float dt)
{
  // Note: this should be closest target
  const auto& first_target = get_first<PlayerComponent>(r);
  if (first_target == entt::null)
    return;
  const auto& first_target_transform = r.get<const TransformComponent>(first_target);

  const auto& view =
    r.view<EnemyComponent, HasTargetPositionComponent, TransformComponent>(entt::exclude<WaitForInitComponent>);
  for (auto [e, enemy, target_position, enemy_t] : view.each()) {

    // Set Target
    auto& targeting = r.get_or_emplace<DynamicTargetComponent>(e);
    targeting.target = first_target;
    target_position.position = first_target_transform.position;

    // Calculate distance
    const auto& other_pos = r.get<TransformComponent>(first_target);
    glm::vec3 dir_raw = other_pos.position - enemy_t.position;
    glm::vec2 dir_nrm = dir_raw;
    if (dir_nrm.x != 0.0f || dir_nrm.y != 0.0f)
      dir_nrm = glm::normalize(dir_nrm);
    const int d2 = dir_raw.x * dir_raw.x + dir_raw.y * dir_raw.y;

    // Components of interest?
    const auto* melee = r.try_get<MeleeComponent>(e);
    const auto* ranged = r.try_get<RangedComponent>(e);

    if (enemy.state == EnemyState::CHASING) {

      // Set as Attacking if within range (Melee)
      if (melee && d2 < melee->distance2) {
        enemy.state = EnemyState::MELEE_ATTACKING;
        r.emplace_or_replace<SeperateTransformFromAABB>(e);
      }

      // Set as Attacking if within shooting range (Ranged)
      if (ranged && d2 < ranged->distance2)
        enemy.state = EnemyState::RANGED_ATTACKING;
    }

    if (enemy.state == EnemyState::MELEE_ATTACKING) {

      // Tick the attack
      if (enemy.attack_percent <= 1.0f) {

        if (enemy.attack_percent >= 0.5f && !enemy.has_applied_damage) {

          // Create a new damage instance
          const auto instance = r.create();
          r.emplace<AttackComponent>(instance, 10);
          r.emplace<TransformComponent>(instance, enemy_t); // copy the parent

          // Send the damage request
          const entt::entity from = instance;
          const entt::entity to = targeting.target;
          r.emplace<DealDamageRequest>(r.create(), from, to);

          enemy.has_applied_damage = true;
        }

        enemy.attack_percent += dt * enemy.attack_speed;
        const float percent = enemy.attack_percent;
        const float interpolation = (-pow(percent, 2) + percent) * 4;

        // Update visuals
        // const glm::vec3 original_position = t.position;
        // const glm::vec3 attack_position = { target_position.position.x, target_position.position.y, 0.0f };
        // const glm::vec3 result = glm::lerp(original_position, attack_position, interpolation);
        // t.position = glm::ivec3(int(result.x), int(result.y), int(result.z));
      }

      // Attack Done!
      if (enemy.attack_percent >= 1.0f) {
        // Reset State
        enemy.attack_percent = 0;
        enemy.has_applied_damage = false;
        enemy.state = EnemyState::CHASING;
        r.remove<SeperateTransformFromAABB>(e);
      }

      //
    }

    if (enemy.state == EnemyState::RANGED_ATTACKING) {
      // Stand your ground!
      target_position.position = enemy_t.position;

      auto& cooldown = r.get<AttackCooldownComponent>(e);

      const bool allowed_to_shoot = !cooldown.on_cooldown;

      if (!allowed_to_shoot) {
        // you shot and you're waiting to shoot again....
      }

      if (allowed_to_shoot) {
        // TODO: audio?

        // put gun on cooldown
        cooldown.on_cooldown = true;
        cooldown.time_between_attack_left = cooldown.time_between_attack;

        // create a bullet at enemy location
        const auto req = create_gameplay(r, EntityType::bullet_enemy);
        r.get<TransformComponent>(req).position = enemy_t.position;
        r.get_or_emplace<HasParentComponent>(req).parent = e;

        auto& bullet_aabb = r.get<AABB>(req);
        bullet_aabb.center = { enemy_t.position.x, enemy_t.position.y };
        auto& bullet_transform = r.get<TransformComponent>(req);
        bullet_transform.rotation_radians.z = enemy_t.rotation_radians.z;

        const float bullet_speed = 100.0f;
        auto& bullet_vel = r.get<VelocityComponent>(req);
        bullet_vel.x = dir_nrm.x * bullet_speed;
        bullet_vel.y = dir_nrm.y * bullet_speed;

        // Turn the bullet Live!
        r.emplace_or_replace<AttackComponent>(req, 3);
        r.emplace_or_replace<EntityTimedLifecycle>(req);

        // Reset State
        enemy.attack_percent = 0;
        enemy.has_applied_damage = false;
        enemy.state = EnemyState::CHASING;
      }
    }
  }
}

// void
// update_enemy_system(entt::registry& r)
// {
//   // check grid exists
//   const auto grid_e = get_first<GridComponent>(r);
//   if (grid_e == entt::null)
//     return;
//   const auto& grid = r.get<GridComponent>(grid_e);

//   // check flowfield exists
//   const auto& field = grid.flow_field;
//   if (field.size() == 0)
//     return; // field not generated yet

//   const auto& view = r.view<const TransformComponent, VelocityComponent, EnemyComponent>();
//   for (const auto& [entity, transform, vel, enemy] : view.each()) {

//     if (!enemy.has_target)
//       update_enemy_get_new_target(grid, transform, enemy);

//     if (enemy.has_target)
//       update_enemy_to_target(transform, enemy, vel);
//   }
// }

} // namespace game2d