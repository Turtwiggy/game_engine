#include "pch.hpp"

#include "spawn_revive_islander_helpers.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/maths/maths.hpp"
#include "modules/actors/actor_boat/boat_components.hpp"
#include "modules/actors/actor_islander/islander_components.hpp"
#include "modules/actors/actor_islander/islander_helpers.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/actors/actor_rock/rock_helpers.hpp"
#include "modules/combat/combat_core/components.hpp"
#include "modules/core/colour/components.hpp"
#include "modules/systems/system_island_nearest/island_nearest_helpers.hpp"
#include "modules/systems/system_island_revive/island_revive_components.hpp"

namespace game2d {

void
handle_death_event__spawn_revive_islander(entt::registry& r, const DeathEvent& evt)
{
  auto dead_e = evt.dead;
  if (!r.all_of<PlayerBoatComponent>(dead_e))
    return;
  if (r.all_of<RevivableComponent>(dead_e))
    return; // you should already have a revive islander

  // spawn a person on the base island.
  auto& player_c = r.get<PlayerComponent>(dead_e);
  auto player_idx = player_c.idx;
  auto colour_idx = player_c.colour_idx;

  // the revive mechanic. spawn a player-islander on the base island.
  auto base_island_e = get_center_island_eid(r);
  auto& base_island_c = r.get<DebugContoursComponent>(base_island_e);

  static engine::RandomState spawn_rnd(0);

  SDL_Log("Spawning a revive islander...");
  auto islander_e =
    spawn_islander_unoccupied_edge(r, spawn_rnd, base_island_e, "actor_islanddweller_player", AvailableTeams::player, false);
  r.emplace<PlayerComponent>(islander_e, PlayerComponent{ .idx = player_idx, .colour_idx = colour_idx });
  auto col = default_player_colours[colour_idx];
  r.emplace_or_replace<DefaultColour>(islander_e, col);
  set_colour(r, islander_e, col);

  r.emplace_or_replace<DroppedAnchorComponent>(dead_e); // add a positional tether to boat
  r.emplace<RevivableComponent>(dead_e);                // add to boat
  r.emplace<RevivableComponent>(islander_e);            // add to islander

  // note: this islander player is deliberately missing components. e.g.
  // InputComponent
}

} // namespace game2d