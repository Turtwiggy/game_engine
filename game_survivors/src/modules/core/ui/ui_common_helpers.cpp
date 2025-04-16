#include "pch.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/events/components.hpp"
#include "engine/events/helpers/keyboard.hpp"
#include "modules/core/fonts/fonts_helpers.hpp"
#include "modules/core/ui/ui_common_components.hpp"
#include "modules/core/ui/ui_common_helpers.hpp"
#include "modules/steam_input/steam_input_components.hpp"
#include "modules/steam_input/steam_input_helpers.hpp"
#include "modules/ui/ui_colours/ui_colours_helpers.hpp"
#include "modules/ui/ui_scene_main_menu_playerjoin/ui_main_menu_playerjoin_components.hpp"
#include "modules/ui/ui_scene_main_menu_playerjoin/ui_main_menu_playerjoin_helpers.hpp"

namespace game2d {

bool
selectable_button(entt::registry& r, SelectableButtonDef& def)
{
  ImGui::PushStyleColor(ImGuiCol_HeaderHovered, IM_COL32(0, 0, 0, 0)); // button hovered
  ImGui::PushStyleColor(ImGuiCol_HeaderActive, IM_COL32(0, 0, 0, 0));  // button clicked

  // const auto& colors = ImGui::GetStyle().Colors;
  // const auto b_col = colors[ImGuiCol_Button];
  // const auto bh_col = colors[ImGuiCol_ButtonActive];
  // const auto ba_col = colors[ImGuiCol_ButtonActive];
  // const ImU32 button_col = IM_COL32(b_col.x * 255, b_col.y * 255, b_col.z * 255, b_col.w * 0);
  // const ImU32 button_hovered_col = IM_COL32(bh_col.x * 255, bh_col.y * 255, bh_col.z * 255, bh_col.w * 0);
  // const ImU32 button_clicked_col = IM_COL32(ba_col.x * 255, ba_col.y * 255, ba_col.z * 255, ba_col.w * 0);

  const auto& size = def.size;

  bool do_act = false;

  // https://github.com/ocornut/imgui/issues/4719
  ImDrawList* draw_list = ImGui::GetWindowDrawList();
  draw_list->ChannelsSplit(2);

  draw_list->ChannelsSetCurrent(1);

  const std::string id = "##menuselectable" + std::to_string(def.my_col_index) + "_" + std::to_string(def.my_row_index);

  ImGui::Selectable(id.c_str(), false, 0, size);

  const ImVec2 mouse_delta = ImGui::GetIO().MouseDelta;
  const bool mouse_move = mouse_delta.x != 0.0f || mouse_delta.y != 0.0f;

  const bool is_hovered = ImGui::IsItemHovered();
  if (is_hovered && mouse_move && def.update_selected_on_mouse_move) {
    def.ui_row_index = def.my_row_index;
    def.ui_col_index = def.my_col_index;
  }

  const bool is_clicked = ImGui::IsItemClicked();
  if (is_clicked) {
    def.ui_row_index = def.my_row_index;
    def.ui_col_index = def.my_col_index;
    do_act = true;
  }

  bool is_selected = def.ui_col_active;
  is_selected &= (def.my_col_index == def.ui_col_index);
  is_selected &= (def.my_row_index == def.ui_row_index);

  if (def.update_selected_only_with_mouse)
    is_selected = is_hovered;

  draw_list->ChannelsSetCurrent(0);
  const auto p_tl = ImGui::GetItemRectMin();
  const auto p_br = ImGui::GetItemRectMax();
  const auto p_wh = ImGui::GetItemRectSize();
  const float rounding = 6.0;
  const float thickness = 2.0;

  const ImU32 im_inactive_outline_col = convert_my_to_im(def.inactive_outline_col);
  const ImU32 im_active_outline_col = convert_my_to_im(def.active_outline_col);
  const ImU32 im_inactive_bg_col = convert_my_to_im(def.inactive_bg_col);
  const ImU32 im_active_bg_col = convert_my_to_im(def.active_bg_col);

  const ImU32 outline_col = is_selected ? im_active_outline_col : im_inactive_outline_col;
  const ImU32 bg_col = is_selected ? im_active_bg_col : im_inactive_bg_col;
  const ImU32 text_col = IM_COL32(255, 255, 255, is_selected ? 255 : 150);

  // button
  draw_list->AddRectFilled(p_tl, p_br, bg_col, rounding);
  draw_list->AddRect(p_tl, p_br, outline_col, rounding, ImDrawFlags_RoundCornersAll, thickness);

  // Drawssome text based on state.
  auto label = def.label;

  auto pos = def.label.find("##");
  if (pos != std::string::npos)
    label = label.substr(0, pos);

  auto font = def.font;
  if (font == nullptr)
    font = ImGui::GetIO().Fonts->Fonts[0];

  const auto text_size = font->CalcTextSizeA(font->FontSize, p_wh.x, -1, label.c_str());
  auto text_pos = ImVec2{ p_tl.x, p_tl.y };

  if (def.text_centered) {
    text_pos.x += 0.5f * (p_wh.x - text_size.x);
    text_pos.y += 0.5f * (p_wh.y - text_size.y);
  }

  text_pos += def.text_offset;
  draw_list->AddText(font, font->FontSize, text_pos, text_col, label.c_str());

  // "commit changes"
  draw_list->ChannelsMerge();

  // play_sound_if_hovered(r, ui.hovered_buttons, label);

  // Do the callback for the button
  // if (is_selected && do_ui_action) {
  //   create_empty<AudioRequestPlayEvent>(r, AudioRequestPlayEvent{ "UI_SELECT_0" });
  //   return true;
  // }

  // input
  if (def.input && is_selected)
    do_act = true;

  ImGui::PopStyleColor(2);
  return do_act;
};

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

  if (state.rows.size() == 0)
    return;

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