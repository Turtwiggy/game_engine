#include "system.hpp"

#include "actors.hpp"
#include "entt/helpers.hpp"
#include "helpers.hpp"
#include "lifecycle/components.hpp"
#include "maths/maths.hpp"
#include "modules/actor_asteroid/components.hpp"
#include "modules/actor_enemy/components.hpp"
#include "modules/actor_enemy_patrol/components.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/actor_weapon_shotgun/components.hpp"
#include "modules/actors/helpers.hpp"
#include "modules/animation/components.hpp"
#include "modules/combat_damage/components.hpp"
#include "modules/combat_wants_to_shoot/components.hpp"
#include "modules/entt/serialize.hpp"
#include "modules/gen_dungeons/components.hpp"
#include "modules/resolve_collisions/helpers.hpp"
#include "modules/scene/components.hpp"
#include "modules/scene/helpers.hpp"
#include "modules/system_particles/components.hpp"
#include "modules/system_particles/helpers.hpp"
#include "modules/ui_combat_turnbased/components.hpp"
#include "physics/components.hpp"

namespace game2d {

void
player_asteroid_collison(entt::registry& r, const entt::entity& a, const entt::entity& b)
{
  const auto [a_ent, b_ent] = collision_of_interest<const PlayerComponent, const AsteroidComponent>(r, a, b);
  if (a_ent != entt::null && b_ent != entt::null) {
    const auto& a_vel = r.get<VelocityComponent>(a_ent);
    const float a_speed = glm::length(glm::vec2{ a_vel.x, a_vel.y });

    const auto& a_pos = get_position(r, a_ent);
    const auto& b_pos = get_position(r, b_ent);
    const auto dir = b_pos - a_pos;

    auto& asteroid_vel = r.get<VelocityComponent>(b_ent);
    asteroid_vel.x = dir.x * a_speed / 10.0f;
    asteroid_vel.y = dir.y * a_speed / 10.0f;

    if (const auto& rotate_around_spot = r.try_get<RotateAroundSpot>(b_ent)) {
      r.remove<RotateAroundSpot>(b_ent);
      r.emplace<EntityTimedLifecycle>(b_ent, 3 * 1000);
    }
  }
}

void
enemy_player_collision(entt::registry& r, const entt::entity& a, const entt::entity& b)
{
  auto& dead = get_first_component<SINGLETON_EntityBinComponent>(r);

  // enemy_player collision for overworld
  const auto& scene = get_first_component<SINGLETON_CurrentScene>(r);
  if (scene.s != Scene::overworld)
    return;

  const auto [a_player, b_group] = collision_of_interest<PlayerComponent, EnemyComponent>(r, a, b);

  if (a_player != entt::null && b_group != entt::null) {

    OverworldToDungeonInfo data;
    data.backstabbed = r.try_get<BackstabbableComponent>(b_group) != nullptr;
    data.patrol_that_you_hit = r.get<PatrolComponent>(b_group);
    create_empty<OverworldToDungeonInfo>(r, data);

    // Destroy the entity we collided with before moving scene,
    // so that when the game is loaded it's gone?
    dead.dead.emplace(b_group);
    r.emplace_or_replace<WaitForInitComponent>(b_group); // set it as not init again

    // save the overworld. TODO: add timestamp to savefile?
    save(r, "save-overworld.json");

    // going to "dungeon" scene
    move_to_scene_start(r, Scene::dungeon_designer);
    create_empty<RequestGenerateDungeonComponent>(r);
  }
}

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

    const auto* a_atk = r.try_get<AttackComponent>(a);
    const auto* a_def = r.try_get<HealthComponent>(a);
    const auto* a_team = r.try_get<TeamComponent>(a);
    const auto* b_atk = r.try_get<AttackComponent>(b);
    const auto* b_def = r.try_get<HealthComponent>(b);
    const auto* b_team = r.try_get<TeamComponent>(b);
    const bool same_team = (a_team && b_team) && (a_team->team == b_team->team);

    // Dealing Damage
    //
    // deal damage to b
    if (a_atk && b_def && !same_team) {
      dead.dead.emplace(a);
      const entt::entity from = a;
      const entt::entity to = b;
      create_empty<DealDamageRequest>(r, DealDamageRequest{ from, to });
    }
    // deal damage to a
    if (b_atk && a_def && !same_team) {
      dead.dead.emplace(b);
      const entt::entity from = b;
      const entt::entity to = a;
      create_empty<DealDamageRequest>(r, DealDamageRequest{ from, to });
    }

    enemy_player_collision(r, a, b);
    player_asteroid_collison(r, a, b);
  }

  for (const Collision2D& coll : physics.frame_solid_collisions) {
    const auto a = static_cast<entt::entity>(coll.ent_id_0);
    const auto b = static_cast<entt::entity>(coll.ent_id_1);

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
        // Set the velocity of the particle
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

        ParticleDescription desc;
        desc.time_to_live_ms = 1000;
        desc.position = { impact_point_x, impact_point_y };
        desc.velocity = { impact_vel_amount_x, impact_vel_amount_y };
        desc.start_size = 10;
        desc.end_size = 0;
        desc.sprite = "EMPTY";
        const auto e = create_particle(r, desc);

        // make it an animation
        // SpriteAnimationComponent anim;
        // anim.playing_animation_name = "SMOKE_IMPACT";
        // anim.duration = desc.time_to_live_ms / 1000;
        // anim.looping = false;
        // r.emplace<SpriteAnimationComponent>(e, anim);
      }
    }
  }
}

} // namespace game2d