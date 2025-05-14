#include "pch.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/maths/maths.hpp"
#include "engine/sprites/helpers.hpp"
#include "modules/actors/actor_player/actor_player_helpers.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/core/renderer/components.hpp"
#include "modules/core/renderer/helpers.hpp"
#include "modules/core/ui/ui_common_components.hpp"
#include "modules/core/ui/ui_common_helpers.hpp"
#include "modules/ui/ui_colours/ui_colours_helpers.hpp"
#include "modules/ui/ui_scene_main_menu_controllerinfo/ui_main_menu_controllerinfo_components.hpp"

namespace game2d {

bool
selectable_button(entt::registry& r, SelectableButtonDef& def)
{
  const auto& ri_c = SINGLE_RendererInfo::instance;

  ImGui::PushStyleColor(ImGuiCol_HeaderHovered, IM_COL32(0, 0, 0, 0)); // button hovered
  ImGui::PushStyleColor(ImGuiCol_HeaderActive, IM_COL32(0, 0, 0, 0));  // button clicked

  // ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
  // ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
  // ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
  // ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);

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

  const ImVec2 mouse_delta = ImGui::GetIO().MouseDelta;
  const bool mouse_move = mouse_delta.x != 0.0f || mouse_delta.y != 0.0f;

  const std::string id = "##menuselectable" + def.cell->name;
  ImGui::Selectable(id.c_str(), false, 0, size);
  const bool is_hovered = ImGui::IsItemHovered();
  const bool is_clicked = ImGui::IsItemClicked();

  if (is_hovered && mouse_move && def.update_selected_on_mouse_move)
    def.active_cell = def.cell;

  if (is_clicked) {
    def.active_cell = def.cell;
    do_act = true;
  }

  bool is_selected = def.active_cell == def.cell;
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
  const ImVec4 text_col_vec = ImVec4(1.0f, 1.0f, 1.0f, is_selected ? 1.0f : 1.0f * (150 / 255.0f));

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

  // draw as an icon
  if (def.icon.has_value()) {
    const auto& icon = def.icon.value();
    const auto tex_id = search_for_texture_id_by_texture_path(ri_c, "custom")->id;
    const auto im_id = reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(tex_id));
    const ImVec2 icon_size{ def.size.x, def.size.y };
    const auto [icon_tl, icon_br] = convert_sprite_to_uv(r, icon);

    const auto icon_padding = 4;
    const auto icon_p_tl = ImVec2{ p_tl.x + icon_padding, p_tl.y + icon_padding };
    const auto icon_p_br = ImVec2{ p_br.x - icon_padding, p_br.y - icon_padding };
    draw_list->AddImage(im_id, icon_p_tl, icon_p_br, icon_tl, icon_br, text_col);
  }

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
  // ImGui::PopStyleVar(4);

  return do_act;
};

void
process_input_for_ui_all_handles(entt::registry& r, UIState& state)
{
  GET_FIRST_OR_RETURN(SINGLE_SteamControllerGameState, r, steam_gs_e, steam_gs_c)

  state.actions.clear();

  const auto input_e = get_first<InputComponent, Persistent>(r);
  const auto& input_c = r.get<InputComponent>(input_e);
  process_input_for_ui(r, state, input_c);
};

void
process_input_for_ui(entt::registry& r, UIState& state, const InputComponent& inp_c)
{
  if (has_action(inp_c.dpad_u, ActionStateEnum::DOWN)) {
    if (state.active->u != nullptr)
      state.active = state.active->u;
    state.actions.push_back(UIAction::NAV_MOVE_U);
  }
  if (has_action(inp_c.dpad_d, ActionStateEnum::DOWN)) {
    if (state.active->d != nullptr)
      state.active = state.active->d;
    state.actions.push_back(UIAction::NAV_MOVE_D);
  }
  if (has_action(inp_c.dpad_l, ActionStateEnum::DOWN)) {
    if (state.active->l != nullptr)
      state.active = state.active->l;
    state.actions.push_back(UIAction::NAV_MOVE_L);
  }
  if (has_action(inp_c.dpad_r, ActionStateEnum::DOWN)) {
    if (state.active->r != nullptr)
      state.active = state.active->r;
    state.actions.push_back(UIAction::NAV_MOVE_R);
  }

  if (has_action(inp_c.button_s, ActionStateEnum::DOWN))
    state.actions.push_back(UIAction::SELECT);

  if (has_action(inp_c.button_e, ActionStateEnum::DOWN))
    state.actions.push_back(UIAction::BACK);
};

void
create_as_vertical_layout(std::vector<std::shared_ptr<Cell>>& cells)
{
  //
  // sort out navigations
  // let every button reference the buttons above/below
  // this is basically a vertical layout group
  //
  for (int i = 0; i < cells.size(); i++) {
    const auto last = engine::wrap(i - 1, (int)cells.size() - 1);
    const auto next = engine::wrap(i + 1, (int)cells.size() - 1);
    const auto& prv = cells[last];
    const auto& nxt = cells[next];
    auto& c = cells[i];
    c->u = prv;
    c->d = nxt;
  }
}

} // namespace game2d