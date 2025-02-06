
#include "modules/controller_input_open_ui/controller_input_open_ui_system.hpp"
#include "engine/entt/helpers.hpp"
#include "modules/steam_input/steam_input_components.hpp"
#include "modules/steam_input/steam_input_helpers.hpp"
#include "modules/ui_popup_pause/ui_popup_pause_components.hpp"

// Note:
// If a controller is unplugged, pause gameplay,
// and display a "please reconnect" screen

namespace game2d {

void
update_controller_input_open_ui_system(entt::registry& r)
{
  GET_FIRST_OR_RETURN(SINGLE_SteamControllers, r, steam_e, steam_c)

  for (int i = 0; i < steam_c.n_active; i++) {
    const auto handle = steam_c.handles[i];

    const auto game_pause = controller_button_down(steam_c, handle, DA::Game_Pause);
    if (game_pause)
      create_empty<RequestToShowPauseMenu>(r);
  }
}

} // namespace game2d
