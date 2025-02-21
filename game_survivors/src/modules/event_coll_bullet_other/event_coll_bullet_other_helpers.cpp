#include "event_coll_bullet_other_helpers.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/maths/maths.hpp"
#include "engine/physics/physics_components.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/combat/components.hpp"
#include "modules/event_coll/event_coll_components.hpp"
#include "modules/event_coll_bullet_other/event_coll_bullet_other_components.hpp"
#include "modules/event_damage/event_damage_components.hpp"
#include "modules/system_traits/trait_components.hpp"
#include "modules/ui_worldspace_text/helpers.hpp"

namespace game2d {

void
handle_bullet_other_coll(entt::registry& r, const OnCollisionEnter& coll_evt)
{
  //
  // Two fixtures have collided,
  // If we choose FixtureOrBody::BODY, check the entt components on the body's user data e, not the fixture.
  // If we choose FixtureOrBody::FIXTURE, check the entt components on the fixture's user data e, not the body.
  //
  const auto [other_e, bullet_e] = collision_of_interest<PhysicsFixtureComponent, BulletComponent>(
    r, coll_evt.a, coll_evt.b, FixtureOrBody::FIXTURE, FixtureOrBody::BODY);

  if (bullet_e == entt::null || other_e == entt::null)
    return;

  // Get the top-level parent, where most of the components will be
  entt::entity other_e_parent = other_e;
  if (auto* has_parent_c = r.try_get<HasParentComponent>(other_e))
    other_e_parent = has_parent_c->parent;

  const auto& parent_tag = r.get<TagComponent>(other_e_parent).tag;
  const auto& fixture_tag = r.get<TagComponent>(other_e).tag;

  const auto& bullet_team_c = r.get<TeamComponent>(bullet_e);
  const auto& other_team_c = r.get<TeamComponent>(other_e_parent);
  if (bullet_team_c.team == other_team_c.team)
    return; // dont damage same team

  // Here, a bullet has collided with something on a different team.
  //

  // Check that the bullet has not collided with this entity before.
  auto& coll = r.get_or_emplace<CollInfo>(bullet_e).other;
  auto it = std::find(coll.begin(), coll.end(), other_e_parent);
  bool bullet_already_coll_with_other = it != coll.end();
  if (bullet_already_coll_with_other)
    return;
  coll.emplace(other_e_parent);

  GET_FIRST_OR_RETURN(SINGLE_Events, r, evts_e, evts_c)

  // note: these values have already been
  // modified with upgrades at the point they were created
  const auto& bullet_traits_c = r.get<TraitComponent>(bullet_e);
  const auto& bullet_damage_c = r.get<BulletDamage>(bullet_e);
  const auto& bullet_knockback_c = r.get<BulletKnockback>(bullet_e);
  auto& bullet_pierce_c = r.get<BulletPierce>(bullet_e);

  auto* hp_c = r.try_get<HealthComponent>(other_e);
  if (hp_c) {
    //
    // Send a damage event from the bullet to the other entity
    //
    {
      DamageEvent evt;
      evt.from = bullet_e;
      evt.to = other_e;
      evt.type = DamageType::PHYSICAL;
      evt.amount = bullet_damage_c.damage;
      evts_c.dispatcher->trigger(evt);
      evts_c.dispatcher->update();
    }

    //
    // give bullets "pierce" as the num enemies you can hit
    //
    {
      bullet_pierce_c.pierced++;
      if (bullet_pierce_c.pierced >= bullet_pierce_c.pierce) {
        // maximum number of enemies pierced with this bullet
        auto& dead = get_first_component<SINGLE_EntityBinComponent>(r);
        dead.dead.emplace(bullet_e);
      }
    }
  }

  auto reverse_velocity = [&r, bullet_e]() {
    auto& bullet_body_c = r.get<PhysicsBodyComponent>(bullet_e);
    bullet_body_c.body->SetLinearVelocity(-1.0 * bullet_body_c.body->GetLinearVelocity());
  };

  // Reverse yo velocity
  // Note: this should work as bullets only collide once with enemies.
  if (auto* bullet_bounce_c = r.try_get<BulletBounce>(bullet_e)) {
    if (bullet_bounce_c->bounces_left > 0) {
      reverse_velocity();
      bullet_bounce_c->bounces_left--;
    }
  } else if (fixture_tag == "shield") {
    reverse_velocity();

    // create a piece of worldspace text saying "blocked" as no damage was given.
    // (unless a health component is ever added to the shield, which it might be)
    if (!hp_c)
      create_popup(r, get_position(r, bullet_e), "blocked!");
  }

  // knockback applies to "core" and "shield"
  if (fixture_tag != "fixture_core" || fixture_tag != "shield")
    return;

  // Knockback the enemy
  if (other_team_c.team == AvailableTeams::enemy) {
    auto& enemy_body_c = r.get<PhysicsBodyComponent>(other_e_parent);
    const auto raw_dir = get_position(r, other_e_parent) - get_position(r, bullet_e);
    const auto nrm_dir = engine::normalize_safe(raw_dir);
    const float knockback_force = bullet_knockback_c.knockback_force;
    enemy_body_c.body->SetLinearVelocity(knockback_force * b2Vec2{ nrm_dir.x, nrm_dir.y });
  }
}

} // namespace game2d