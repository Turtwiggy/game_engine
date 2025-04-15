#include "pch.hpp"

#include "engine/entt/helpers.hpp"
#include "modules/combat/combat_core/components.hpp"
#include "modules/events/event_death/components.hpp"
#include "stats_components.hpp"

namespace game2d {

void
handle_death_event__update_stats(entt::registry& r, const DeathEvent& evt)
{
  GET_FIRST_OR_RETURN(SINGLE_SurviveStatsComponent, r, data_e, data_c)

  // something ded.
  if (auto* dead_team_c = r.try_get<TeamComponent>(evt.dead)) {
    if (dead_team_c->team == AvailableTeams::enemy)
      data_c.enemies_killed += 1;
  }
}

} // namespace game2d