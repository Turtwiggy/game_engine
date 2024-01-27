#include "system.hpp"

#include "entt/helpers.hpp"
#include "modules/actor_enemy/components.hpp"
#include "modules/actor_player/components.hpp"
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

  const auto& view = r.view<EnemyComponent, HasTargetPositionComponent>(entt::exclude<WaitForInitComponent>);
  for (auto [e, enemy, target_position] : view.each()) {
    auto& t = r.get<TransformComponent>(e);

    // Set Target
    auto& targeting = r.get_or_emplace<DynamicTargetComponent>(e);
    targeting.target = first_target;
    target_position.position = first_target_transform.position;

    if (enemy.state == EnemyState::CHASING) {
      // Calculate distance
      const auto& other = first_target;
      const auto& other_pos = r.get<TransformComponent>(other);
      const auto d = t.position - other_pos.position;
      const int d2 = d.x * d.x + d.y * d.y;

      // Set as Attacking if within range (Melee)
      constexpr float sprite_width = 16;
      constexpr float min_distance = sprite_width * sprite_width + sprite_width * sprite_width;
      if (d2 < min_distance) {
        enemy.state = EnemyState::ATTACKING;
        r.emplace_or_replace<SeperateTransformFromAABB>(e);
      }

      // Set as Attacking if within shooting range (Ranged)
      // TODO...
    }

    // Do Attack Stuff?
    if (enemy.state == EnemyState::ATTACKING) {
      target_position.position = t.position;

      // Tick the attack
      if (enemy.attack_percent <= 1.0f) {

        if (enemy.attack_percent >= 0.5f && !enemy.has_applied_damage) {

          // Create a new damage instance
          const auto instance = r.create();
          r.emplace<AttackComponent>(instance, 10);
          r.emplace<TransformComponent>(instance, t); // copy the parent

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