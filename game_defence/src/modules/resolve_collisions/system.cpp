#include "system.hpp"

#include "actors.hpp"
#include "entt/helpers.hpp"
#include "maths/maths.hpp"
#include "modules/actor_enemy/components.hpp"
#include "modules/actor_particle/components.hpp"
#include "modules/actor_weapon_shotgun/components.hpp"
#include "modules/combat_damage/components.hpp"
#include "modules/combat_wants_to_shoot/components.hpp"
#include "modules/lifecycle/components.hpp"
#include "modules/renderer/helpers.hpp"
#include "modules/resolve_collisions/helpers.hpp"
#include "physics/components.hpp"
#include "renderer/transform.hpp"
#include "sprites/components.hpp"

namespace game2d {

void
update_resolve_collisions_system(entt::registry& r)
{
  static engine::RandomState rnd;
  const auto& physics = get_first_component<SINGLETON_PhysicsComponent>(r);

  // some collisions result in dead entities
  auto& dead = get_first_component<SINGLETON_EntityBinComponent>(r);

  for (const auto& coll : physics.collision_enter) {

    const auto a = static_cast<entt::entity>(coll.ent_id_0);
    const auto b = static_cast<entt::entity>(coll.ent_id_1);

    const auto& a_type = r.get<EntityTypeComponent>(a).type;
    const auto& b_type = r.get<EntityTypeComponent>(b).type;

    const auto* a_atk = r.try_get<AttackComponent>(a);
    const auto* a_def = r.try_get<HealthComponent>(a);
    const auto* a_team = r.try_get<TeamComponent>(a);
    const auto* b_atk = r.try_get<AttackComponent>(b);
    const auto* b_def = r.try_get<HealthComponent>(b);
    const auto* b_team = r.try_get<TeamComponent>(b);

    // Dealing Damage
    //
    // deal damage to b
    if (a_atk && b_def && a_team && b_team) {
      if (a_team->team == b_team->team)
        continue; // no team damage
      dead.dead.emplace(a);
      const entt::entity from = a;
      const entt::entity to = b;
      r.emplace<DealDamageRequest>(r.create(), from, to);
    }
    // deal damage to a
    if (b_atk && a_def && a_team && b_team) {
      if (a_team->team == b_team->team)
        continue; // no team damage
      dead.dead.emplace(b);
      const entt::entity from = b;
      const entt::entity to = a;
      r.emplace<DealDamageRequest>(r.create(), from, to);
    }
  }

  for (const auto& coll : physics.collision_stay) {

    const auto a = static_cast<entt::entity>(coll.ent_id_0);
    const auto b = static_cast<entt::entity>(coll.ent_id_1);

    const auto& a_type = r.get<EntityTypeComponent>(a).type;
    const auto& b_type = r.get<EntityTypeComponent>(b).type;

    // check if an enemy is colling with player line of sight
    //
    const auto [a_ent, b_ent] = collision_of_interest<const LineOfSightComponent, const EnemyComponent>(r, a, b);
    if (a_ent != entt::null && b_ent != entt::null) {
      // an enemy is colliding with line of sight!
      const auto& parent = r.get<HasParentComponent>(a_ent); // type player
      r.emplace_or_replace<WantsToShoot>(parent.parent);
    }

    // MERGE the ENEMIES!
    //
    // const auto [a_ent_1, b_ent_1] = collision_of_interest<const EnemyComponent, const EnemyComponent>(r, a, b);
    // if (a_ent_1 != entt::null && b_ent_1 != entt::null)
    //   dead.dead.emplace(a_ent_1);
  }

  for (const Collision2D& coll : physics.frame_solid_collisions) {
    const auto a = static_cast<entt::entity>(coll.ent_id_0);
    const auto b = static_cast<entt::entity>(coll.ent_id_1);
    const auto& a_type = r.get<EntityTypeComponent>(a).type;
    const auto& b_type = r.get<EntityTypeComponent>(b).type;

    const auto [a_ent, b_ent] = collision_of_interest<const BulletComponent, const EntityTypeComponent>(r, a, b);
    if (a_ent != entt::null && b_ent != entt::null) {
      const auto& bullet = a_ent;
      const auto& wall = b_ent;

      // get rid of bullet
      dead.dead.emplace(bullet);

      // which direction was the bullet moving in?
      const auto& bullet_vel = r.get<VelocityComponent>(bullet);
      const auto& bullet_aabb = r.get<AABB>(bullet);
      const float sign_x = glm::sign(bullet_vel.x);
      const float sign_y = glm::sign(bullet_vel.y);
      const auto impact_point_x = bullet_aabb.center.x + (sign_x * (bullet_aabb.size.x / 2.0f));
      const auto impact_point_y = bullet_aabb.center.y + (sign_y * (bullet_aabb.size.y / 2.0f));

      // todo: 6 dust clouds
      // todo: 3 sparks
      // todo: spawn permanant debris. look in to gamemaker surface system?
      // todo: slow the particle down over time
      // todo: give the particle a limited range to bounce off at with some randomness

      // root particle for collision
      {
        const float particle_seconds_to_live = 1;
        const auto coll_e = create_gameplay(r, EntityType::particle);
        auto& life = r.get<EntityTimedLifecycle>(coll_e);
        life.milliseconds_alive_max = particle_seconds_to_live * 1000;

        auto& collpoint_t = r.get<TransformComponent>(coll_e);
        collpoint_t.position = { impact_point_x, impact_point_y, 0.0f };
        // choose a random rotation for the sprite to be less same-y
        collpoint_t.rotation_radians.z = engine::rand_det_s(rnd.rng, 0.0f, 2.0f * engine::PI);

        // Set the velocity of the particle
        //
        float impact_vel_amount_x = 0.0f;
        float impact_vel_amount_y = 0.0f;
        const float momentum_loss = 5.0f; // e.g. particles return at 1/5th the speed
        // going more horizontal than vertical
        // just set impact-vel as perpendicular to the impact point
        if (glm::abs(bullet_vel.x) >= glm::abs(bullet_vel.y))
          impact_vel_amount_x = (-bullet_vel.x) / momentum_loss;
        // going more vertical than horizontal
        // just set impact-vel as perpendicular to the impact point
        else
          impact_vel_amount_y = (-bullet_vel.y) / momentum_loss;
        auto& collpoint_vel = r.get<VelocityComponent>(coll_e);
        collpoint_vel = { impact_vel_amount_x, impact_vel_amount_y };

        // update particle sprite to the correct sprite
        auto sc = create_sprite(r, "SMOKE_IMPACT", EntityType::particle);
        r.emplace_or_replace<SpriteComponent>(coll_e, sc);

        // make it an animation
        SpriteAnimationComponent anim;
        anim.playing_animation_name = "SMOKE_IMPACT";
        anim.duration = particle_seconds_to_live;
        anim.looping = false;
        r.emplace<SpriteAnimationComponent>(coll_e, anim);

        // make it shrink
        ScaleOverTimeComponent sotc;
        sotc.seconds_until_complete = particle_seconds_to_live;
        sotc.start_size = 10.0f;
        sotc.end_size = 0.0f;
        r.emplace<ScaleOverTimeComponent>(coll_e, sotc);
      }
    }
  }
}

} // namespace game2d