#include "controller_input_update_ui_helpers.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/events/components.hpp"
#include "engine/events/helpers/keyboard.hpp"
#include "modules/steam_input/steam_input_components.hpp"
#include "modules/steam_input/steam_input_helpers.hpp"
#include "modules/ui_scene_main_menu_playerjoin/ui_main_menu_playerjoin_components.hpp"
#include "modules/ui_scene_main_menu_playerjoin/ui_main_menu_playerjoin_helpers.hpp"

#include <entt/entt.hpp>

namespace game2d {

void
process_input_for_ui(entt::registry& r, UIState& state)
{
  GET_FIRST_OR_RETURN(SINGLE_SteamControllerGameState, r, steam_gs_e, steam_gs_c)
  GET_FIRST_OR_RETURN(SINGLE_SteamControllers, r, steam_e, steam_c)

  // state
  int& selected = state.selected;
  bool& do_ui_action = state.do_action;
  const int max = state.max;

  //
  // Update menu via controller
  //
  const auto nz_handles = non_zero_handles(steam_gs_c.handles);
  for (int i = 0; i < (int)nz_handles.size(); i++) {
    const auto handle = nz_handles[i];
    if (handle_joined_this_frame(steam_gs_c, handle))
      continue; // prevent immediately doing do_ui_action

    if (controller_button_down(steam_c, handle, DA::Game_Up))
      selected--;
    if (controller_button_down(steam_c, handle, DA::Game_Down))
      selected++;
    if (controller_button_down(steam_c, handle, DA::Game_Select))
      do_ui_action = true;
  }

  //
  // Update menu via keyboard (debug, mostly)
  //
  GET_FIRST_OR_RETURN(SINGLE_InputComponent, r, input_e, input)

  if (get_key_down(input, SDL_SCANCODE_KP_MINUS))
    selected--;
  if (get_key_down(input, SDL_SCANCODE_KP_PLUS))
    selected++;
  if (get_key_down(input, SDL_SCANCODE_KP_ENTER))
    do_ui_action = true;

  // clamp selected
  selected = selected < 0 ? max - 1 : selected;
  selected %= max;
}

} // namespace game2d