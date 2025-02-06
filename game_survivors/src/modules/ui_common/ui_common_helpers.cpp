#include "modules/ui_common/ui_common_helpers.hpp"

namespace game2d {

bool
selectable_button(SelectableButtonDef& def)
{
  const auto& colors = ImGui::GetStyle().Colors;
  const auto b = ImGuiCol_Button;
  const auto bh = ImGuiCol_ButtonHovered;
  const auto ba = ImGuiCol_ButtonActive;
  const auto b_col = colors[b];
  const auto bh_col = colors[bh];
  const auto ba_col = colors[ba];
  const ImU32 button_col = IM_COL32(b_col.x * 255, b_col.y * 255, b_col.z * 255, b_col.w * 255);
  const ImU32 button_hovered_col = IM_COL32(bh_col.x * 255, bh_col.y * 255, bh_col.z * 255, bh_col.w * 255);
  const ImU32 button_clicked_col = IM_COL32(ba_col.x * 255, ba_col.y * 255, ba_col.z * 255, ba_col.w * 255);
  const ImU32 button_outline_col = button_hovered_col;

  const auto& label = def.label;
  const auto& size = def.size;
  const auto index = def.index;
  bool do_act = false;

  // https://github.com/ocornut/imgui/issues/4719
  ImDrawList* draw_list = ImGui::GetWindowDrawList();
  draw_list->ChannelsSplit(2);

  draw_list->ChannelsSetCurrent(1);

  ImGui::Selectable(label.c_str(), false, 0, size);

  const bool is_hovered = ImGui::IsItemHovered();
  if (is_hovered)
    def.sel_index = def.index;

  const bool is_clicked = ImGui::IsItemClicked();
  if (is_clicked) {
    def.sel_index = def.index;
    do_act = true;
  }

  const bool is_selected = def.sel_index == def.index;

  draw_list->ChannelsSetCurrent(0);
  const auto p_min = ImGui::GetItemRectMin();
  const auto p_max = ImGui::GetItemRectMax();
  const float rounding = 6.0;
  const float thickness = 2.0;
  const ImDrawFlags corners = ImDrawFlags_RoundCornersTopLeft | ImDrawFlags_RoundCornersBottomRight;

  // button background based on state
  if (is_clicked)
    ImGui::GetWindowDrawList()->AddRectFilled(p_min, p_max, button_clicked_col, rounding);
  else if (is_hovered)
    ImGui::GetWindowDrawList()->AddRectFilled(p_min, p_max, button_hovered_col, rounding);
  else
    ImGui::GetWindowDrawList()->AddRectFilled(p_min, p_max, button_col, rounding);

  // button outline
  if (is_selected)
    ImGui::GetWindowDrawList()->AddRect(p_min, p_max, button_outline_col, rounding, corners, thickness);

  // "commit changes"
  draw_list->ChannelsMerge();

  // play_sound_if_hovered(r, ui.hovered_buttons, label);

  // Do the callback for the button
  // if (is_selected && do_ui_action) {
  //   create_empty<AudioRequestPlayEvent>(r, AudioRequestPlayEvent{ "UI_SELECT_01" });
  //   return true;
  // }

  // consume input
  if (def.input && is_selected) {
    do_act = true;
    def.input = false; // consume
  }

  return do_act;
};

} // namespace game2d