#include "event_coll_bullet_enemy_helpers.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/maths/maths.hpp"
#include "engine/physics/components.hpp"
#include "modules/combat/components.hpp"
#include "modules/event_Damage/event_damage_components.hpp"
#include "modules/system_autofire/autofire_components.hpp"
#include "modules/system_traits/trait_components.hpp"

namespace game2d {

void
handle_bullet_enemy_coll(entt::registry& r, const OnCollisionEnter& coll_evt)
{
  //
  // Two fixtures have collided,
  // If we choose FixtureOrBody::BODY, check the entt components on the body's user data e, not the fixture.
  // If we choose FixtureOrBody::FIXTURE, check the entt components on the fixture's user data e, not the body.
  //
  const auto [bullet_e, team_e] = collision_of_interest<BulletComponent, TeamComponent>(
    r, coll_evt.a, coll_evt.b, FixtureOrBody::BODY, FixtureOrBody::BODY);

  if (bullet_e == entt::null || team_e == entt::null)
    return;
  if (r.get<TeamComponent>(bullet_e).team != AvailableTeams::player)
    return;
  if (r.get<TeamComponent>(team_e).team != AvailableTeams::enemy)
    return;
  GET_FIRST_OR_RETURN(SINGLE_Events, r, evts_e, evts_c)

  auto& bullet_c = r.get<BulletComponent>(bullet_e);
  auto& traits_c = r.get<TraitComponent>(bullet_e);

  DamageEvent evt;
  evt.from = entt::null; // bullet likely ded next frame
  evt.to = team_e;
  evt.type = DamageType::PHYSICAL;
  evt.amount = bullet_c.damage;
  evt.traits = traits_c.traits;
  evts_c.dispatcher->trigger(evt);
  evts_c.dispatcher->update();

  // TODO: give bullets "pierce" as the num enemies you can hit
  auto& dead = get_first_component<SINGLE_EntityBinComponent>(r);
  dead.dead.emplace(bullet_e);

  // Slightly knockback the enemy
  auto& enemy_body_c = r.get<PhysicsBodyComponent>(team_e);
  const auto raw_dir = get_position(r, team_e) - get_position(r, bullet_e);
  const auto nrm_dir = engine::normalize_safe(raw_dir);
  // const float knockback_amount = 25000.0f;
  // enemy_body_c.body->ApplyLinearImpulseToCenter({ nrm_dir.x * knockback_amount, nrm_dir.y * knockback_amount }, true);
  const float knockback_amount = 50;
  enemy_body_c.body->SetLinearVelocity(knockback_amount * b2Vec2{ nrm_dir.x, nrm_dir.y });
}

} // namespace game2d