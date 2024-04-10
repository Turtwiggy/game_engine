#include "system.hpp"

#include "entt/helpers.hpp"
#include "lifecycle/components.hpp"
#include "maths/grid.hpp"
#include "maths/maths.hpp"
#include "modules/actor_enemy/components.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/ai_pathfinding/components.hpp"
#include "modules/ai_pathfinding/helpers.hpp"
#include "modules/combat_attack_cooldown/components.hpp"
#include "modules/combat_damage/components.hpp"
#include "modules/combat_wants_to_shoot/components.hpp"
#include "modules/lerp_to_target/components.hpp"
#include "modules/selected_interactions/components.hpp"
#include "physics/components.hpp"
#include "renderer/transform.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/compatibility.hpp> // lerp

namespace game2d {

void
update_enemy_system(entt::registry& r, const float dt)
{
  //
  // Update enemies to preferred velocity.
  //
  // note: this should probably take in to account
  // pathfinding, such as using AStar to calculate the next step
  //

  const auto& map = get_first_component<MapComponent>(r);

  // Convert Map to Grid (?)
  GridComponent grid;
  grid.size = map.tilesize;
  grid.width = map.xmax;
  grid.height = map.ymax;
  grid.grid = map.map;

  const auto& default_target = get_first<PlayerComponent>(r);
  const auto& default_target_aabb = r.get<AABB>(default_target);
  const auto& view =
    r.view<EnemyComponent, const HasTargetPositionComponent, TransformComponent, AABB>(entt::exclude<WaitForInitComponent>);

  const auto convert_position_to_index = [&map](const glm::ivec2& src) -> int {
    auto src_gridpos = engine::grid::world_space_to_grid_space(src, map.tilesize);
    src_gridpos.x = glm::clamp(src_gridpos.x, 0, map.xmax - 1);
    src_gridpos.y = glm::clamp(src_gridpos.y, 0, map.ymax - 1);
    return engine::grid::grid_position_to_index(src_gridpos, map.xmax);
  };

  for (const auto& [e, enemy, target_position, enemy_t, aabb] : view.each()) {

    const auto src = aabb.center;
    const auto dst = default_target_aabb.center;
    const int src_idx = convert_position_to_index(src);
    const int dst_idx = convert_position_to_index(dst);

    const auto update_pathfinding = [&r, &e, &grid, &src_idx, &dst_idx, &src, &dst, &default_target]() {
      const auto path = generate_direct(r, grid, src_idx, dst_idx);
      GeneratedPathComponent path_c;
      path_c.path = path;
      path_c.src_pos = src;
      path_c.dst_pos = dst;
      path_c.dst_ent = default_target;
      path_c.path_cleared.resize(path.size());
      r.emplace_or_replace<GeneratedPathComponent>(e, path_c);
    };

    // Attach a GeneratedPath to the enemy unit. HasTargetPosition gets overwritten.
    const auto& existing_path = r.try_get<GeneratedPathComponent>(e);
    if (existing_path == NULL)
      update_pathfinding();
    else {
      // When to update path? Only if target moves?
      const auto existing_idx = convert_position_to_index(existing_path->dst_pos);
      if (existing_idx != dst_idx)
        update_pathfinding();
    }

    // Calculate distance
    const auto tgt_as_vec2 = glm::vec2(target_position.position.x, target_position.position.y);
    const auto dir_raw = tgt_as_vec2 - glm::vec2(aabb.center);
    const auto dir_nrm = engine::normalize_safe({ dir_raw.x, dir_raw.y });
    const int d2 = dir_raw.x * dir_raw.x + dir_raw.y * dir_raw.y;

    // Components of interest?
    const auto* melee = r.try_get<MeleeComponent>(e);
    const auto* ranged = r.try_get<RangedComponent>(e);

    if (enemy.state == EnemyState::CHASING) {
      // Set as Attacking if within range (Melee)
      if (melee && d2 < melee->distance2) {
        enemy.state = EnemyState::MELEE_ATTACKING;
      }

      // Set as Attacking if within shooting range (Ranged)
      if (ranged && d2 < ranged->distance2)
        enemy.state = EnemyState::RANGED_ATTACKING;
    }

    if (enemy.state == EnemyState::MELEE_ATTACKING) {
      // const auto& targeting = r.get<DynamicTargetComponent>(e);

      // Tick the attack
      if (enemy.attack_percent <= 1.0F) {

        if (enemy.attack_percent >= 0.5F && !enemy.has_applied_damage) {

          // Create a new damage instance
          const auto instance = r.create();
          r.emplace<AttackComponent>(instance, 10);
          r.emplace<TransformComponent>(instance, enemy_t); // copy the parent

          // Send the damage request
          // const entt::entity from = instance;
          // const entt::entity to = targeting.target;
          // r.emplace<DealDamageRequest>(r.create(), from, to);

          enemy.has_applied_damage = true;
        }

        enemy.attack_percent += dt * enemy.attack_speed;
        // const float percent = enemy.attack_percent;
        // const float interpolation = (-pow(percent, 2) + percent) * 4;

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
        // r.remove<SeperateTransformFromAABB>(e);
      }
      //
    }

    if (enemy.state == EnemyState::RANGED_ATTACKING) {
      // Stand your ground!
      // target_position.position = enemy_t.position;

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

  //
}
} // namespace game2d