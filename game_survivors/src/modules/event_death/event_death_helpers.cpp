#include "event_death_helpers.hpp"

#include "engine/entt/helpers.hpp"
#include "modules/ui_units/ui_units_components.hpp"
#include "modules/ui_units/ui_units_helpers.hpp"

namespace game2d {

void
handle_death_event_set_unit_as_dead(entt::registry& r, const DeathEvent& evt)
{
  // auto& units = get_first_component<SINGLE_LoadedUnits>(r).units;

  // find the unit that died
  // set it as perma dead
  SDL_Log("TODO: impl killing units");

  // save_units(r);
};

} // namespace game2d