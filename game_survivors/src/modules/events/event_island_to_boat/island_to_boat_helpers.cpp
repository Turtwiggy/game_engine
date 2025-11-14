#include "pch.hpp"

#include "island_to_boat_helpers.hpp"

#include "engine/entt/helpers.hpp"
#include "modules/systems/system_spawner/spawner_system.hpp"
#include "modules/ui/ui_scene_survive_timer/ui_survive_timer_components.hpp"

namespace game2d {

void
handle_island_to_boat_event__start_game(entt::registry& r, const IslandToBoatEvent& evt)
{
  GET_FIRST_OR_RETURN(SurviveTimerComponent, r, survive_e, survive_c);

  // survive_c.game_started = true;
}

} // namespace game2d