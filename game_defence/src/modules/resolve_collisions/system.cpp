#include "system.hpp"

#include "entt/helpers.hpp"
#include "lifecycle/components.hpp"
#include "modules/combat_damage/components.hpp"
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

    // const auto [bbouncy_ent, some_ent] =
    //   collision_of_interest<const BulletBouncyComponent, const EntityTypeComponent>(r, a, b);
    // if (bbouncy_ent != entt::null && some_ent != entt::null) {
    //   const auto& bullet_e = bbouncy_ent;
    //   const auto& solid_e = some_ent;
    //   const auto& bullet_aabb = r.get<AABB>(bullet_e);
    //   const auto& solid_aabb = r.get<AABB>(solid_e);

    //   // bouncy bullet hit a wall
    //   const bool wall_is_horizontal = solid_aabb.size.x > solid_aabb.size.y;
    //   const auto dir = bullet_aabb.center - solid_aabb.center;

    //   // move the bullet away from the collided wall
    //   auto& bullet_vel = r.get<VelocityComponent>(bullet_e);
    //   if (wall_is_horizontal) // reflect y
    //     bullet_vel.y = glm::abs(bullet_vel.y) * glm::sign(dir.y);
    //   else
    //     bullet_vel.x = glm::abs(bullet_vel.x) * glm::sign(dir.x);
    // }

    // const auto [a_ent, b_ent] = collision_of_interest<const BulletComponent, const EntityTypeComponent>(r, a, b);
    //     if (a_ent != entt::null && b_ent != entt::null) {
    //       const auto& bullet = a_ent;
    //       const auto& wall = b_ent;

    //       // get rid of bullet
    //       dead.dead.emplace(bullet);

    //       // which direction was the bullet moving in?
    //       const auto& bullet_vel = r.get<VelocityComponent>(bullet);
    //       const auto& bullet_aabb = r.get<AABB>(bullet);
    //       const auto& solid_aabb = r.get<AABB>(wall);
    //       const float sign_x = glm::sign(bullet_vel.x);
    //       const float sign_y = glm::sign(bullet_vel.y);
    //       const auto impact_point_x = bullet_aabb.center.x + (sign_x * (bullet_aabb.size.x / 2.0f));
    //       const auto impact_point_y = bullet_aabb.center.y + (sign_y * (bullet_aabb.size.y / 2.0f));

    //       // todo: 6 dust clouds
    //       // todo: 3 sparks
    //       // todo: spawn permanant debris. look in to gamemaker surface system?
    //       // todo: slow the particle down over time
    //       // todo: give the particle a limited range to bounce off at with some randomness

    //       // root particle for collision
    //       {
    //         // Set the velocity of the particle
    //         float impact_vel_amount_x = 0.0f;
    //         float impact_vel_amount_y = 0.0f;
    //         const float momentum_loss = 5.0f; // e.g. particles return at 1/5th the speed

    //         // bouncy bullet hit a wall
    //         const bool wall_is_horizontal = solid_aabb.size.x > solid_aabb.size.y;
    //         const auto dir = bullet_aabb.center - solid_aabb.center;

    //         if (wall_is_horizontal)
    //           impact_vel_amount_y = glm::abs(bullet_vel.y) * glm::sign(dir.y) / momentum_loss;
    //         else
    //           impact_vel_amount_x = glm::abs(bullet_vel.x) * glm::sign(dir.x) / momentum_loss;

    //         ParticleDescription desc;
    //         desc.time_to_live_ms = 1000;
    //         desc.position = { impact_point_x, impact_point_y };
    //         desc.velocity = { impact_vel_amount_x, impact_vel_amount_y };
    //         desc.start_size = 10;
    //         desc.end_size = 0;
    //         desc.sprite = "EMPTY";
    //         const auto e = create_particle(r, desc);

    //         // make it an animation
    //         // SpriteAnimationComponent anim;
    //         // anim.playing_animation_name = "SMOKE_IMPACT";
    //         // anim.duration = desc.time_to_live_ms / 1000;
    //         // anim.looping = false;
    //         // r.emplace<SpriteAnimationComponent>(e, anim);
    //       }
    //     }
  }
}

} // namespace game2d