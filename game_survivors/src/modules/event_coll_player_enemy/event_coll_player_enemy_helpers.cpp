#include "event_coll_player_enemy_helpers.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/maths/maths.hpp"
#include "engine/physics/components.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/combat/components.hpp"
#include "modules/event_coll/event_coll_components.hpp"
#include "modules/event_damage/event_damage_components.hpp"
#include "modules/raws/raws_components.hpp"
#include "modules/resolve_collisions/resolve_collisions_helpers.hpp"
#include "modules/system_autofire/autofire_components.hpp"

namespace game2d {

void
handle_player_enemy_coll_enter(entt::registry& r, const OnCollisionEnter& coll_evt)
{
  //
  // Two fixtures have collided,
  // If we choose FixtureOrBody::BODY, check the entt components on the body's user data e, not the fixture.
  // If we choose FixtureOrBody::FIXTURE, check the entt components on the fixture's user data e, not the body.
  //

  // PlayerFixtureComponent will be on the fixture level
  // TeamComponent will be on the body level
  const auto [player_fixture_e, enemy_e] = collision_of_interest<PlayerFixtureComponent, TeamComponent>(
    r, coll_evt.a, coll_evt.b, FixtureOrBody::FIXTURE, FixtureOrBody::BODY);

  if (player_fixture_e == entt::null || enemy_e == entt::null)
    return;

  if (r.get<TeamComponent>(enemy_e).team != AvailableTeams::enemy)
    return;

  auto player_e = r.get<HasParentComponent>(player_fixture_e).parent;

  // if the enemy is already colliding with the player, dont damage again
  auto& coll = r.get_or_emplace<CollInfo>(enemy_e).other;
  auto it = std::find(coll.begin(), coll.end(), player_e);
  if (it != coll.end())
    return;
  coll.emplace(player_e); // new coll

  GET_FIRST_OR_RETURN(SINGLE_Events, r, evts_e, evts_c)

  // You collided with a bullet, not an enemy
  // Destroy the bullet, damage the player
  if (auto* bullet_c = r.try_get<BulletComponent>(enemy_e)) {

    // destroy bullet
    auto& dead = get_first_component<SINGLE_EntityBinComponent>(r);
    dead.dead.emplace(enemy_e);

    // damage player
    DamageEvent evt;
    evt.from = enemy_e;
    evt.to = player_e;
    evt.type = DamageType::PHYSICAL;
    evt.amount = 1; // TODO: fix bullet_c->damage damage;
    evt.traits = {};
    evts_c.dispatcher->trigger(evt);
    evts_c.dispatcher->update();

    return;
  }

  // Note, for enemies, checking the item template, not the live TraitComponent
  const auto key = r.get<ItemKey>(enemy_e).key;
  const auto item_c = find_item(r, key);

  std::string exlosive_trait = "explode";
  if (item_c.traits.has_value()) {
    auto find_trait = [&](const Trait& t) { return t.key == exlosive_trait; };
    auto it = std::find_if(item_c.traits.value().begin(), item_c.traits.value().end(), find_trait);
    if (it != item_c.traits.value().end()) {
      //
      // You've collided with an enemy that explodes
      // Kill the enemy, and that should trigger the
      // explode-on-death callback
      //
      const auto& hp_c = r.get<HealthComponent>(enemy_e);

      DamageEvent evt;
      evt.from = entt::null;
      evt.to = enemy_e;
      evt.type = DamageType::PHYSICAL;
      evt.amount = hp_c.max_hp;
      evt.traits = {};
      evts_c.dispatcher->trigger(evt);
      evts_c.dispatcher->update();

      return;
    }
  }

  DamageEvent evt;
  evt.from = enemy_e;
  evt.to = player_e;
  evt.type = DamageType::PHYSICAL;
  evt.amount = 1; // todo: replace with "correct" damage for enemy
  evt.traits = {};
  evts_c.dispatcher->trigger(evt);
  evts_c.dispatcher->update();

  // Apply a force in the direction away from the player to the enemy
  const auto dir = engine::normalize_safe(get_position(r, enemy_e) - get_position(r, player_e));
  const auto& enemy_phys = r.get<PhysicsBodyComponent>(enemy_e);

  // ding ding, you hit. now stop and move away
  // This sets the velocity this frame,
  // but then gets taken over by physics again
  enemy_phys.body->SetLinearVelocity(50.0f * b2Vec2{ dir.x, dir.y });
};

void
handle_player_enemy_coll_exit(entt::registry& r, const OnCollisionExit& coll_evt)
{
  const auto [player_e, enemy_e] = collision_of_interest<PlayerFixtureComponent, TeamComponent>(
    r, coll_evt.a, coll_evt.b, FixtureOrBody::FIXTURE, FixtureOrBody::BODY);

  if (player_e == entt::null || enemy_e == entt::null)
    return;
  if (r.get<TeamComponent>(enemy_e).team != AvailableTeams::enemy)
    return;

  // The enemy has left the player. remove the player from it.
  auto& coll = r.get_or_emplace<CollInfo>(enemy_e);
  const auto& it = std::find(coll.other.begin(), coll.other.end(), player_e);
  if (it != coll.other.end())
    coll.other.erase(it);

  //
};

} // namespace game2d