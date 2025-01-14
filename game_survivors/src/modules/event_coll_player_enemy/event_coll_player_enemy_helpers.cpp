#include "event_coll_player_enemy_helpers.hpp"
#include "engine/actors/actor_helpers.hpp"
#include "engine/maths/maths.hpp"
#include "engine/physics/components.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/combat/components.hpp"
#include "modules/event_coll_player_item/event_coll_player_item_helpers.hpp"
#include "modules/event_damage/event_damage_components.hpp"

namespace game2d {

void
handle_player_enemy_coll_enter(entt::registry& r, const OnCollisionEnter& coll_evt)
{
  const auto [player_e, enemy_e] = collision_of_interest<PlayerComponent, TeamComponent>(r, coll_evt.a, coll_evt.b);
  if (player_e == entt::null || enemy_e == entt::null)
    return;
  if (r.get<TeamComponent>(enemy_e).team != AvailableTeams::enemy)
    return;

  // if the enemy is already colliding with the player, dont damage again
  auto& coll = r.get_or_emplace<CollInfo>(enemy_e).other;
  auto it = std::find(coll.begin(), coll.end(), player_e);
  if (it != coll.end())
    return;
  coll.emplace(player_e); // new coll

  GET_FIRST_OR_RETURN(SINGLE_Events, r, evts_e, evts_c)

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
  enemy_phys.body->SetLinearVelocity(50.0f * b2Vec2{ dir.x, dir.y });
};

void
handle_player_enemy_coll_exit(entt::registry& r, const OnCollisionExit& coll_evt)
{
  const auto [player_e, enemy_e] = collision_of_interest<PlayerComponent, TeamComponent>(r, coll_evt.a, coll_evt.b);
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