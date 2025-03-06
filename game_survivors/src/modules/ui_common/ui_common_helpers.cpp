#include "modules/ui_common/ui_common_helpers.hpp"
#include "imgui.h"
#include "modules/ui_colours/ui_colours_helpers.hpp"

namespace game2d {

const auto my_active_col = hex_to_srgb("#FFFFFF", 255);
const auto im_active_col = convert_my_to_im(my_active_col);

const auto my_inactive_col = hex_to_srgb("#FFFFFF", 0.6 * 255);
const auto im_inactive_col = convert_my_to_im(my_inactive_col);

const auto my_active_button_background_col = hex_to_srgb("#02526D", 255);
const auto im_active_button_background_col = convert_my_to_im(my_active_button_background_col);

const auto my_inactive_button_background_col = hex_to_srgb("#02526D", 0.6 * 255);
const auto im_inactive_button_background_col = convert_my_to_im(my_inactive_button_background_col);

bool
selectable_button(SelectableButtonDef& def)
{
  ImGui::PushStyleColor(ImGuiCol_HeaderHovered, IM_COL32(0, 0, 0, 0)); // button hovered
  ImGui::PushStyleColor(ImGuiCol_HeaderActive, IM_COL32(0, 0, 0, 0));  // button clicked

  const auto& colors = ImGui::GetStyle().Colors;
  const auto b_col = colors[ImGuiCol_Button];
  const auto bh_col = colors[ImGuiCol_ButtonActive];
  const auto ba_col = colors[ImGuiCol_ButtonActive];
  const ImU32 button_col = IM_COL32(b_col.x * 255, b_col.y * 255, b_col.z * 255, b_col.w * 0);
  const ImU32 button_hovered_col = IM_COL32(bh_col.x * 255, bh_col.y * 255, bh_col.z * 255, bh_col.w * 0);
  const ImU32 button_clicked_col = IM_COL32(ba_col.x * 255, ba_col.y * 255, ba_col.z * 255, ba_col.w * 0);

  const auto& size = def.size;
  const auto index = def.index;
  bool do_act = false;

  // https://github.com/ocornut/imgui/issues/4719
  ImDrawList* draw_list = ImGui::GetWindowDrawList();
  draw_list->ChannelsSplit(2);

  draw_list->ChannelsSetCurrent(1);

  const std::string id = "##menuselectable" + std::to_string(def.index);
  ImGui::Selectable(id.c_str(), false, 0, size);

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
  const auto p_size = ImGui::GetItemRectSize();
  const float rounding = 8.0;
  const float thickness = 2.0;
  const ImDrawFlags corners = ImDrawFlags_RoundCornersAll;

  ImU32 colour = im_inactive_col;
  if (is_selected && is_hovered)
    colour = im_active_col;
  else if (is_selected)
    colour = im_active_col;

  ImU32 button_background_col = im_inactive_button_background_col;
  if (is_selected && is_hovered)
    button_background_col = im_active_button_background_col;
  else if (is_selected)
    button_background_col = im_active_button_background_col;

  // button background based on state
  // if (is_clicked)
  //   ImGui::GetWindowDrawList()->AddRectFilled(p_min, p_max, button_clicked_col, rounding);
  // else if (is_hovered)
  //   ImGui::GetWindowDrawList()->AddRectFilled(p_min, p_max, button_hovered_col, rounding);
  // else
  ImGui::GetWindowDrawList()->AddRectFilled(p_min, p_max, button_background_col, rounding);

  // button outline
  ImGui::GetWindowDrawList()->AddRect(p_min, p_max, colour, rounding, corners, thickness);

  // Draw some text based on state.
  auto label = def.label;
  auto pos = def.label.find("##");
  if (pos != std::string::npos)
    label = label.substr(0, pos);

  const auto text_size = ImGui::CalcTextSize(label.c_str());
  const auto text_pos = ImVec2{
    p_min.x + 0.5f * (p_size.x - text_size.x),
    p_min.y + 0.5f * (p_size.y - text_size.y),
  };
  ImGui::GetWindowDrawList()->AddText(text_pos, colour, label.c_str());

  // "commit changes"
  draw_list->ChannelsMerge();

  // play_sound_if_hovered(r, ui.hovered_buttons, label);

  // Do the callback for the button
  // if (is_selected && do_ui_action) {
  //   create_empty<AudioRequestPlayEvent>(r, AudioRequestPlayEvent{ "UI_SELECT_0" });
  //   return true;
  // }

  // consume input
  if (def.input && is_selected) {
    do_act = true;
    def.input = false; // consume
  }

  ImGui::PopStyleColor(2);
  return do_act;
};

} // namespace game2d