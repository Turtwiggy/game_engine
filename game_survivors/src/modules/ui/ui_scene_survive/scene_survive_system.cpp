#include "scene_survive_system.hpp"

#include "engine/entt/helpers.hpp"
#include "modules/steam_input/steam_input_components.hpp"
#include "modules/steam_input/steam_input_helpers.hpp"

namespace game2d {

void
update_ui_scene_survive_system(entt::registry& r)
{
  GET_FIRST_OR_RETURN(SINGLE_SteamControllers, r, steam_e, steam_c)

  // Set the action set now we're in-game
  set_all_steam_controller_action_set(steam_c, ActionSet::ActionSet_GameControls);
}

} // namespace game2d