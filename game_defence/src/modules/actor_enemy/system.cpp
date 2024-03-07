#include "system.hpp"

#include "entt/helpers.hpp"
#include "helpers/line.hpp"
#include "maths/maths.hpp"
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

glm::vec2
abs_squared(const glm::vec2& a)
{
  return a * a;
}

//
//  Try to perform reciprical velocity obstacle (RVO) avoidance
//  https://gamma.cs.unc.edu/HRVO/HRVO-T-RO.pdf
//
void
update_velocity_based_on_rvo()
{
  // const auto& vel_view = r.view<const AABB, VelocityComponent, const EnemyComponent>();

  // for (const auto& [e, enemy, target_position, enemy_t, vel, aabb] : view.each()) {

  // future optimisation: we do not need to take all the other agents in to account when
  // selecting a new velocity, as the penalty of the velocities will
  // not depend on all agents that are far away.

  // {
  //   for (const auto& [b_e, b_aabb, b_vel, b_enemy] : vel_view.each()) {
  //     if (e == b_e)
  //       continue; // avoid yourself, bruh

  //     const auto pos_a = glm::vec2(aabb.center.x, aabb.center.y);     // x: 100
  //     const auto vel_a = glm::vec2(vel.x, vel.y);                     // x: -10
  //     const auto pos_b = glm::vec2(b_aabb.center.x, b_aabb.center.y); // x: -100
  //     const auto vel_b = glm::vec2(b_vel.x, b_vel.y);                 // x: 10
  //     const auto relative_pos = pos_b - pos_a;                        // -200, 0

  //     // assume circles for the minkowski sum
  //     const auto combined_radius = ((aabb.size.x / 2.0f) + (b_aabb.size.x / 2.0f)) * 2.0f; // 32

  //     // if this dot product is positive, they're heading towards eachother
  //     // if its negative, they're heading away from each other (no possible collision)
  //     //
  //     const auto relative_vel = vel_a - vel_b; // -10, 0
  //     const auto dot_product = glm::dot(relative_pos, relative_vel);
  //     ImGui::Text("DotProduct: %f", dot_product);
  //     if (dot_product < 0)
  //       continue; // not headed for a collision, continue along preferred velocity#

  //     // warning: time to collision doesnt take in to account radius
  //     // const auto speed = glm::length(relative_vel);
  //     // const auto time_to_collision = dot_product / (speed * speed);
  //     // ImGui::Text("ttc: %f", time_to_collision);

  //     // Calculate the Velocity Obstacle
  //     //
  //     const auto distance = glm::length(relative_pos);
  //     const auto angle = atan2(relative_pos.y, relative_pos.x); // angle to x-axis
  //     const auto tangent_angle = atan(combined_radius / distance);
  //     const auto tangent_angle_left = angle + tangent_angle;
  //     const auto tangent_angle_right = angle - tangent_angle;
  //     auto tangent_point_left = pos_a;
  //     tangent_point_left.x += cos(tangent_angle_left) * distance;
  //     tangent_point_left.y += sin(tangent_angle_left) * distance;
  //     auto tangent_point_right = pos_a;
  //     tangent_point_right.x += cos(tangent_angle_right) * distance;
  //     tangent_point_right.y += sin(tangent_angle_right) * distance;
  //     // TODO: calculate the far side of the velocity object

  //     // Translate the VO in to a RVO.
  //     //
  //     const glm::vec2 translation = (vel_a + vel_b) / 2.0f;
  //     const auto a1 = pos_a + translation;
  //     const auto a2 = tangent_point_left + translation;
  //     const auto b1 = pos_a + translation;
  //     const auto b2 = tangent_point_right + translation;
  //     const auto c1 = tangent_point_left + translation;  // should be far side
  //     const auto c2 = tangent_point_right + translation; // should be far side
  //     // set_transform_with_line(r.get<TransformComponent>(enemy.cone_l), generate_line(a1, a2, 2));
  //     // set_transform_with_line(r.get<TransformComponent>(enemy.cone_r), generate_line(b1, b2, 2));
  //     // set_transform_with_line(r.get<TransformComponent>(enemy.cone_far), generate_line(c1, c2, 2));

  //     // Pick a velocity outside the RVO
  //     //
  //     const auto dir_left = glm::normalize(a2 - a1);
  //     const auto dir_right = glm::normalize(b2 - b1);
  //     const auto nrm_vel = glm::normalize(vel_a);

  //     // Is the current velocity already outside the RVO?
  //     //
  //     const auto valid_vel_check_a = glm::dot(nrm_vel, dir_left) < 0;
  //     const auto valid_vel_check_b = glm::dot(nrm_vel, dir_right) < 0;
  //     if (valid_vel_check_a && valid_vel_check_b)
  //       continue; // no need to adjust velocity, velocity is fine

  //     // Otherwise, pick a new velocity outside of the RVO.
  //     //
  //     // When choosing a new velocity,
  //     // the average is taken of its current velocity
  //     // and a velocity outside the veloicty object.
  //     // Robots are typically required to select the velocity closest to their own preferred velocity.

  //     // TODO...
  //     vel.x = dir_left.x * vel.base_speed;
  //     vel.y = dir_left.y * vel.base_speed;
  //   }
}

void
update_enemy_system(entt::registry& r, const float dt)
{
  // Note: this should be closest target
  const auto& first_target = get_first<PlayerComponent>(r);
  if (first_target == entt::null)
    return;
  const auto& first_target_transform = r.get<const TransformComponent>(first_target);

  const auto& view = r.view<EnemyComponent, HasTargetPositionComponent, TransformComponent, VelocityComponent, AABB>(
    entt::exclude<WaitForInitComponent>);

  //
  // Update enemies to preferred velocity.
  //
  // note: this should probably take in to account
  // pathfinding, such as using AStar to calculate the next step
  //
  for (const auto& [e, enemy, target_position, enemy_t, vel, aabb] : view.each()) {

    // Set Target to player?
    auto& targeting = r.get_or_emplace<DynamicTargetComponent>(e);
    targeting.target = first_target;
    target_position.position = first_target_transform.position;

    // in this case, the preferred velocity is a vector in the direction of the goal
    const glm::ivec2 a = { aabb.center.x, aabb.center.y };
    const glm::ivec2 b = target_position.position;

    const glm::vec2 raw_v = b - a;
    const glm::vec2 nrm_v = engine::normalize_safe(raw_v);

    // vel.preferred_x = desired_v.x * vel.base_speed;
    // vel.preferred_y = desired_v.y * vel.base_speed;
    // vel.x = vel.preferred_x;
    // vel.y = vel.preferred_y;
    vel.x = nrm_v.x * vel.base_speed;
    vel.y = nrm_v.y * vel.base_speed;

    // }
    //
    // Update AI Actions
    //
    // for (const auto& [e, enemy, target_position, enemy_t, vel, aabb] : view.each()) {

    // Calculate distance
    const auto& other_pos = r.get<TransformComponent>(first_target);
    const glm::vec3 dir_raw = other_pos.position - enemy_t.position;
    const glm::vec2 dir_nrm = engine::normalize_safe({ dir_raw.x, dir_raw.y });
    const int d2 = dir_raw.x * dir_raw.x + dir_raw.y * dir_raw.y;

    // Components of interest?
    const auto* melee = r.try_get<MeleeComponent>(e);
    const auto* ranged = r.try_get<RangedComponent>(e);

    if (enemy.state == EnemyState::CHASING) {

      // Set as Attacking if within range (Melee)
      if (melee && d2 < melee->distance2) {
        enemy.state = EnemyState::MELEE_ATTACKING;
        // r.emplace_or_replace<SeperateTransformFromAABB>(e);
      }

      // Set as Attacking if within shooting range (Ranged)
      if (ranged && d2 < ranged->distance2)
        enemy.state = EnemyState::RANGED_ATTACKING;
    }

    if (enemy.state == EnemyState::MELEE_ATTACKING) {
      const auto& targeting = r.get<DynamicTargetComponent>(e);

      // Tick the attack
      if (enemy.attack_percent <= 1.0F) {

        if (enemy.attack_percent >= 0.5F && !enemy.has_applied_damage) {

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

  //
}
} // namespace game2d