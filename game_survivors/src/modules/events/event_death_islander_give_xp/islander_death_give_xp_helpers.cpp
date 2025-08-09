#include "pch.hpp"

#include "islander_death_give_xp_helpers.hpp"

#include "engine/entt/helpers.hpp"
#include "modules/combat/combat_core/components.hpp"
#include "modules/events/event_coll_player_xp/event_coll_player_xp_components.hpp"
#include "modules/systems/system_island_movement/island_movement_components.hpp"

namespace game2d {

void
handle_death_event__islander_death_give_xp(entt::registry& r, const DeathEvent& evt)
{
  const auto dead_e = evt.dead;

  // only remove islanders
  if (!r.all_of<MovementIslandComponent>(dead_e))
    return;

  // only give xp by killing enemies
  if (!r.all_of<TeamComponent>(dead_e))
    return;
  auto& team_c = r.get<TeamComponent>(dead_e);
  if (team_c.team != AvailableTeams::enemy)
    return;

  // give an xp. could make it so that islanders have an xp amount. dunno!
  auto& sxp_c = get_first_component<SINGLE_XpComponent>(r);
  sxp_c.xp++;
}

} // namespace game2d