#include "pch.hpp"

#include "engine/lifecycle/components.hpp"
#include "engine/physics/physics_components.hpp"
#include "event_coll_player_island.hpp"
#include "modules/actors/actor_islanddweller/islanddweller_components.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/actors/actor_rock/rock_components.hpp"
#include "modules/systems/system_island_return_to_boat_land_immunity/island_return_to_boat_land_immunity_components.hpp"

namespace game2d {

void
handle_player_enter_island(entt::registry& r, const OnCollisionEnter& evt)
{
  return; // disabled

  const auto [pfixture_e, fix_island_e] = coll<PlayerFixtureComponent, IslandFixtureComponent>(r, evt.a, evt.b);
  if (pfixture_e == entt::null || fix_island_e == entt::null)
    return;

  const auto par_e = r.get<HasParentComponent>(pfixture_e).parent;

  const auto* dropped_anchor_c = r.try_get<DroppedAnchorComponent>(par_e);
  if (dropped_anchor_c)
    return; // your boat is already on the island.

  const auto island_e = r.get<HasParentComponent>(fix_island_e).parent;
  const auto* immunity_c = r.try_get<IslandCollisionImmunity>(par_e);
  if (immunity_c && immunity_c->island_e == fix_island_e)
    return; // your boat has a cooldown before landing again

  // SDL_Log("PlayerCollidedWithIsland");

  // stop the boats momentum when you land.
  b2Body_SetLinearVelocity(r.get<PhysicsBodyComponent>(par_e).bodyId, { 0, 0 });

  // add a component to the player
  r.emplace_or_replace<PlayerCollidedWithIsland>(par_e);

  // search for "remove_hidden_state_from_island"
}

} // namespace game2d