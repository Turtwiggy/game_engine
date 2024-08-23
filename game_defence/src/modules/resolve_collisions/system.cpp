#include "system.hpp"

#include "actors/actors.hpp"
#include "entt/helpers.hpp"
#include "lifecycle/components.hpp"
#include "modules/actor_weapon_shotgun/components.hpp"
#include "modules/actors/helpers.hpp"
#include "modules/combat_damage/components.hpp"
#include "modules/resolve_collisions/helpers.hpp"
#include "physics/components.hpp"

#include <box2d/box2d.h>

namespace game2d {

void
update_resolve_collisions_system(entt::registry& r)
{
  auto& physics = get_first_component<SINGLE_Physics>(r);
  auto& world = physics.world;

  // some collisions result in dead entities
  auto& dead = get_first_component<SINGLETON_EntityBinComponent>(r);

  for (b2Contact* contact = world->GetContactList(); contact; contact = contact->GetNext()) {
    const entt::entity a = (entt::entity)contact->GetFixtureA()->GetBody()->GetUserData().pointer;
    const entt::entity b = (entt::entity)contact->GetFixtureB()->GetBody()->GetUserData().pointer;

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
    // if (a_atk && b_def && !same_team) {
    if (a_atk && b_def) {
      dead.dead.emplace(a);
      const entt::entity from = a;
      const entt::entity to = b;
      create_empty<DealDamageRequest>(r, DealDamageRequest{ from, to });
    }
    // deal damage to a
    // if (b_atk && a_def && !same_team) {
    if (b_atk && a_def) {
      dead.dead.emplace(b);
      const entt::entity from = b;
      const entt::entity to = a;
      create_empty<DealDamageRequest>(r, DealDamageRequest{ from, to });
    }

    const auto [a_ent, b_ent] = collision_of_interest<const BulletComponent, const EntityTypeComponent>(r, a, b);
    if (a_ent != entt::null && b_ent != entt::null) {
      const auto bullet_e = a_ent;
      const auto wall_e = b_ent;

      const auto& bullet_body = r.get<PhysicsBodyComponent>(bullet_e);
      const auto bullet_pos = get_position(r, bullet_e);
      const auto bullet_size = get_size(r, bullet_e);
      const auto& bullet_vel = bullet_body.body->GetLinearVelocity();

      const auto& b_type = r.get<EntityTypeComponent>(wall_e);
      // const auto& solid_body = r.get<PhysicsBodyComponent>(wall_e);
      const auto solid_pos = get_position(r, wall_e);
      const auto solid_size = get_size(r, wall_e);

      const auto& info = r.get<BulletComponent>(a_ent);

      // if hit person, destroy bullet
      if (b_type.type == EntityType::actor_dungeon && info.destroy_bullet_on_actor_collision)
        dead.dead.emplace(bullet_e);

      if (b_type.type != EntityType::solid_wall)
        continue; // walls only from here on out

      // bouncy bullet hit a wall
      //
      if (info.bounce_bullet_on_wall_collision) {
        const bool wall_is_horizontal = solid_size.x > solid_size.y;
        const auto dir = bullet_pos - solid_pos;

        // move the bullet away from the collided wall

        const auto existing_vel = bullet_body.body->GetLinearVelocity();
        if (wall_is_horizontal) // reflect y
          bullet_body.body->SetLinearVelocity({ existing_vel.x, glm::abs(existing_vel.y) * glm::sign(dir.y) });
        else // reflect x
          bullet_body.body->SetLinearVelocity({ glm::abs(existing_vel.x) * glm::sign(dir.x), existing_vel.y });
      }

      // get rid of bullet
      //
      if (info.destroy_bullet_on_wall_collision)
        dead.dead.emplace(bullet_e);

      // spawn collison particle effects
      //
      const float sign_x = glm::sign(bullet_vel.x);
      const float sign_y = glm::sign(bullet_vel.y);
      const auto impact_point_x = bullet_pos.x + (sign_x * (bullet_size.x / 2.0f));
      const auto impact_point_y = bullet_pos.y + (sign_y * (bullet_size.y / 2.0f));

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

        // bouncy bullet hit a wall
        const bool wall_is_horizontal = solid_size.x > solid_size.y;
        const auto dir = bullet_pos - solid_pos;

        if (wall_is_horizontal)
          impact_vel_amount_y = glm::abs(bullet_vel.y) * glm::sign(dir.y) / momentum_loss;
        else
          impact_vel_amount_x = glm::abs(bullet_vel.x) * glm::sign(dir.x) / momentum_loss;

        Particle desc;
        desc.pos = { impact_point_x, impact_point_y };
        desc.time_to_live_ms = 1000;
        desc.velocity = { impact_vel_amount_x, impact_vel_amount_y };
        desc.start_size = 6;
        desc.end_size = 2;
        desc.sprite = "EMPTY";
        const auto e = Factory_Particle::create(r, desc);

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