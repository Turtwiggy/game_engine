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
check_if_changed(UIState& state, int v_selected, int h_selected)
{
  const bool v_changed = v_selected != state.current_row_index;
  const bool h_changed = h_selected != state.rows[v_selected].col_index;

  // vertical changed...
  if (v_changed) {
    state.actions.push_back(UIAction::V_VALUE_CHANGED);

    if (v_selected < state.current_row_index)
      state.actions.push_back(UIAction::V_VALUE_CHANGED_UP);
    if (v_selected > state.current_row_index)
      state.actions.push_back(UIAction::V_VALUE_CHANGED_DOWN);
  }

  // horizontal changed...
  if (!v_changed && h_changed) {
    state.actions.push_back(UIAction::H_VALUE_CHANGED);

    if (h_selected < state.rows[v_selected].col_index)
      state.actions.push_back(UIAction::H_VALUE_CHANGED_RIGHT);
    if (h_selected > state.rows[v_selected].col_index)
      state.actions.push_back(UIAction::H_VALUE_CHANGED_LEFT);
  }

  // clamp selected
  const int max = state.rows.size();
  if (max == 0)
    throw std::runtime_error("Error: no rows in ui. Probably setup incorrect.");

  state.current_row_index = state.current_row_index < 0 ? max - 1 : state.current_row_index;
  state.current_row_index %= max;
};

void
process_keyboard_input_for_ui(entt::registry& r, UIState& state)
{
  GET_FIRST_OR_RETURN(SINGLE_InputComponent, r, input_e, input)

  int v_selected = state.current_row_index;
  int h_selected = state.rows[v_selected].col_index;

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
      state.actions.push_back(UIAction::SELECT);
    else if (get_key_down(input, SDL_SCANCODE_KP_ENTER))
      state.actions.push_back(UIAction::SELECT);
    else if (get_key_down(input, SDL_SCANCODE_ESCAPE))
      state.actions.push_back(UIAction::BACK);
    else if (get_key_down(input, SDL_SCANCODE_BACKSPACE))
      state.actions.push_back(UIAction::BACK);
  }

  check_if_changed(state, v_selected, h_selected);
}

void
process_input_for_ui_all_handles(entt::registry& r, UIState& state)
{
  GET_FIRST_OR_RETURN(SINGLE_SteamControllerGameState, r, steam_gs_e, steam_gs_c)

  state.actions.clear();

  process_keyboard_input_for_ui(r, state);

  for (int i = 0; i < steam_gs_c.handles.size(); i++)
    process_input_for_ui(r, state, steam_gs_c.handles[i]);
};

void
process_input_for_ui(entt::registry& r, UIState& state, const InputHandle_t handle)
{
  GET_FIRST_OR_RETURN(SINGLE_SteamControllers, r, steam_e, steam_c)
  GET_FIRST_OR_RETURN(SINGLE_SteamControllerGameState, r, steam_gs_e, steam_gs_c)

  if (handle_joined_this_frame(steam_gs_c, handle))
    return; // prevent immediately doing do_ui_action

  // TODO: replace this system with has_action, which maps both keyboard and controller

  // state
  int v_selected = state.current_row_index;
  int h_selected = state.rows[v_selected].col_index;

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
    state.actions.push_back(UIAction::SELECT);
  else if (controller_button_down(steam_c, handle, DA::Game_East))
    state.actions.push_back(UIAction::BACK);

  check_if_changed(state, v_selected, h_selected);
};

} // namespace game2d