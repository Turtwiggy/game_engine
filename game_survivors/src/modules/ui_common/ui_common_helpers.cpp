#include "pch.hpp"

#include "engine/entt/helpers.hpp"
#include "modules/core_fonts/fonts_helpers.hpp"
#include "modules/ui_colours/ui_colours_helpers.hpp"
#include "modules/ui_common/ui_common_helpers.hpp"

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

} // namespace game2d