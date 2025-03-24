#include "pch.hpp"

#include "controller_input_update_ui_helpers.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/events/components.hpp"
#include "engine/events/helpers/keyboard.hpp"
#include "modules/steam_input/steam_input_components.hpp"
#include "modules/steam_input/steam_input_helpers.hpp"
#include "modules/ui_common/ui_common_components.hpp"
#include "modules/ui_scene_main_menu_playerjoin/ui_main_menu_playerjoin_components.hpp"
#include "modules/ui_scene_main_menu_playerjoin/ui_main_menu_playerjoin_helpers.hpp"

namespace game2d {

void
process_input_for_ui_all_handles(entt::registry& r, UIState& state)
{
  GET_FIRST_OR_RETURN(SINGLE_SteamControllerGameState, r, steam_gs_e, steam_gs_c)
  GET_FIRST_OR_RETURN(SINGLE_SteamControllers, r, steam_e, steam_c)

  const auto nz_handles = non_zero_handles(steam_gs_c.handles);

  state.new_actions.clear();
  for (int i = 0; i < (int)nz_handles.size(); i++) {
    auto handle = nz_handles[i];
    const bool connected = handle_is_connected(steam_c, handle);
    const bool joined = handle_is_joined(steam_gs_c, handle);

    if (joined && connected)
      process_input_for_ui(r, state, handle);
  }
};

void
process_input_for_ui(entt::registry& r, UIState& state, const InputHandle_t handle)
{
  GET_FIRST_OR_RETURN(SINGLE_SteamControllers, r, steam_e, steam_c)
  GET_FIRST_OR_RETURN(SINGLE_SteamControllerGameState, r, steam_gs_e, steam_gs_c)
  GET_FIRST_OR_RETURN(SINGLE_InputComponent, r, input_e, input)

  if (handle == 0)
    return;
  if (handle_joined_this_frame(steam_gs_c, handle))
    return; // prevent immediately doing do_ui_action

  // state
  int v_selected = state.current_row_index;
  int h_selected = state.rows[v_selected].col_index;

  // TODO: replace this system with has_action, which maps both keyboard and controller

  // Update menu via controller
  //
  if (controller_button_down(steam_c, handle, DA::Game_Up))
    state.current_row_index--;
  else if (controller_button_down(steam_c, handle, DA::Game_Down))
    state.current_row_index++;
  else if (controller_button_down(steam_c, handle, DA::Game_Left))
    state.rows[v_selected].col_index--;
  else if (controller_button_down(steam_c, handle, DA::Game_Right))
    state.rows[v_selected].col_index++;
  else if (controller_button_down(steam_c, handle, DA::Game_South))
    state.new_actions.push_back(UIAction::SELECT);
  else if (controller_button_down(steam_c, handle, DA::Game_East))
    state.new_actions.push_back(UIAction::BACK);

  // Update menu via keyboard (debug, mostly)
  //
  {
    if (get_key_down(input, SDL_SCANCODE_UP))
      state.current_row_index--;
    else if (get_key_down(input, SDL_SCANCODE_DOWN))
      state.current_row_index++;
    else if (get_key_down(input, SDL_SCANCODE_LEFT))
      state.rows[v_selected].col_index--;
    else if (get_key_down(input, SDL_SCANCODE_RIGHT))
      state.rows[v_selected].col_index++;
    else if (get_key_down(input, SDL_SCANCODE_RETURN))
      state.new_actions.push_back(UIAction::SELECT);
    else if (get_key_down(input, SDL_SCANCODE_KP_DECIMAL))
      state.new_actions.push_back(UIAction::BACK);
  }

  const bool v_changed = v_selected != state.current_row_index;
  const bool h_changed = h_selected != state.rows[v_selected].col_index;

  // vertical changed...
  if (v_changed)
    state.new_actions.push_back(UIAction::V_VALUE_CHANGED);

  // horizontal value changed...
  if (!v_changed && h_changed)
    state.new_actions.push_back(UIAction::H_VALUE_CHANGED);

  // clamp selected
  const int max = state.rows.size();
  if (max == 0)
    throw std::runtime_error("Error: no rows in ui. Probably setup incorrect.");

  state.current_row_index = state.current_row_index < 0 ? max - 1 : state.current_row_index;
  state.current_row_index %= max;
}

} // namespace game2d