#include "system.hpp"

#include "actors/actors.hpp"
#include "actors/helpers.hpp"
#include "entt/helpers.hpp"
#include "lifecycle/components.hpp"
#include "modules/actor_cover/components.hpp"
#include "modules/actor_weapon_shotgun/components.hpp"
#include "modules/combat_damage/components.hpp"
#include "modules/combat_flash_on_damage/components.hpp"
#include "modules/combat_scale_on_hit/components.hpp"
#include "modules/resolve_collisions/helpers.hpp"
#include "physics/components.hpp"

#include <box2d/b2_body.h>
#include <box2d/box2d.h>

namespace game2d {

void
update_resolve_collisions_system(entt::registry& r)
{
  const auto& physics = get_first_component<SINGLE_Physics>(r);
  const auto& world = physics.world;

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

      // TEMPORARY: !same_team check.
      // enemies are currently shooting eachother
      // in the back of the head. need to implement a
      // line of sight check to fix that.
      if (!same_team) {
        const entt::entity from = a;
        const entt::entity to = b;
        create_empty<DealDamageRequest>(r, DealDamageRequest{ from, to });
      }
    }
    // deal damage to a
    // if (b_atk && a_def && !same_team) {
    if (b_atk && a_def) {
      dead.dead.emplace(b);

      // TEMPORARY: !same_team check.
      // enemies are currently shooting eachother
      // in the back of the head. need to implement a
      // line of sight check to fix that.
      if (!same_team) {
        const entt::entity from = b;
        const entt::entity to = a;
        create_empty<DealDamageRequest>(r, DealDamageRequest{ from, to });
      }
    }

    const auto [a_ent, b_ent] = collision_of_interest<const BulletComponent, const PhysicsBodyComponent>(r, a, b);
    if (a_ent != entt::null && b_ent != entt::null) {
      const auto bullet_e = a_ent;
      const auto other_e = b_ent;

      const auto& bullet_body = r.get<PhysicsBodyComponent>(bullet_e);
      const auto bullet_pos = get_position(r, bullet_e);
      const auto bullet_size = get_size(r, bullet_e);
      const auto& bullet_vel = bullet_body.body->GetLinearVelocity();
      const auto& bullet_info = r.get<BulletComponent>(bullet_e);

      const auto& other_body = r.get<PhysicsBodyComponent>(other_e);
      const auto other_pos = get_position(r, other_e);
      const auto other_size = get_size(r, other_e);
      const bool is_wall = other_body.body->GetType() == b2_staticBody;

      // check if the bullet should be destroyed by the other entity (e.g. wall, enemy)
      if (auto* destroy_bullet_req = r.try_get<DestroyBulletOnCollison>(other_e))
        dead.dead.emplace(bullet_e);

      if (auto* req = r.try_get<CoverComponent>(other_e)) {

        // WARNING: this attackidcomponent logic here is duplicate
        // with the quip system

        // get the attack id of the bullet
        const auto* atk_id = r.try_get<AttackIdComponent>(bullet_e);

        const uint64_t atk_unique_id = atk_id->id;
        auto& hit_list = r.get<DefenceHitListComponent>(other_e);
        auto& hit_list_attack_ids_taken = hit_list.attack_id_taken;

        auto it = std::find(hit_list_attack_ids_taken.begin(), hit_list_attack_ids_taken.end(), atk_unique_id);
        if (it != hit_list_attack_ids_taken.end()) {
          // fmt::println("atk_id already taken...");
          continue;
        }

        // we've not taken damage from this attack id before!
        hit_list_attack_ids_taken.push_back(atk_unique_id);

        // Do the work....
        {
          // get the cover's transform to "pop" in size
          if (r.try_get<RequestHitScaleComponent>(other_e) == nullptr)
            r.emplace<RequestHitScaleComponent>(other_e);

          // get the cover to flash
          if (r.try_get<RequestFlashComponent>(other_e) == nullptr)
            r.emplace<RequestFlashComponent>(other_e);
        }

        // continue; // dont spawn particles or bounce on cover
      }

      if (!is_wall)
        continue; // walls only from here on out

      // bouncy bullet hit a wall
      //
      if (bullet_info.bounce_bullet_on_wall_collision) {
        const bool wall_is_horizontal = other_size.x > other_size.y;
        const auto dir = bullet_pos - other_pos;

        // move the bullet away from the collided wall
        const auto existing_vel = bullet_body.body->GetLinearVelocity();
        if (wall_is_horizontal) // reflect y
          bullet_body.body->SetLinearVelocity({ existing_vel.x, glm::abs(existing_vel.y) * glm::sign(dir.y) });
        else // reflect x
          bullet_body.body->SetLinearVelocity({ glm::abs(existing_vel.x) * glm::sign(dir.x), existing_vel.y });
      }

      if (const auto* req = r.try_get<RequestParticleOnCollision>(other_e)) {

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
          const bool wall_is_horizontal = other_size.x > other_size.y;
          const auto dir = bullet_pos - other_pos;

          if (wall_is_horizontal)
            impact_vel_amount_y = glm::abs(bullet_vel.y) * glm::sign(dir.y) / momentum_loss;
          else
            impact_vel_amount_x = glm::abs(bullet_vel.x) * glm::sign(dir.x) / momentum_loss;

          DataParticle desc;
          desc.pos = { impact_point_x, impact_point_y };
          desc.time_to_live_ms = 1000;
          desc.velocity = { impact_vel_amount_x, impact_vel_amount_y };
          desc.start_size = 6;
          desc.end_size = 2;
          desc.sprite = "EMPTY";
          const auto e = Factory_DataParticle::create(r, desc);

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
}

} // namespace game2d