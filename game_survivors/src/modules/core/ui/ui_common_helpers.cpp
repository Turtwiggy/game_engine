#include "pch.hpp"

#include "engine/audio/audio_components.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/maths/maths.hpp"
#include "engine/sprites/helpers.hpp"
#include "engine/std/vector/helpers.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/core/renderer/components.hpp"
#include "modules/core/renderer/helpers.hpp"
#include "modules/core/ui/ui_common_components.hpp"
#include "modules/core/ui/ui_common_helpers.hpp"
#include "modules/steam_input/steam_input_components.hpp"
#include "modules/ui/ui_colours/ui_colours_helpers.hpp"
#include "modules/ui/ui_scene_main_menu_controllerinfo/ui_main_menu_controllerinfo_components.hpp"

namespace game2d {
using namespace std::literals;

bool
selectable_button(entt::registry& r, SelectableButtonDef& def)
{
  const auto& ri_c = SINGLE_RendererInfo::instance;

  ImGui::PushStyleColor(ImGuiCol_HeaderHovered, IM_COL32(0, 0, 0, 0)); // button hovered
  ImGui::PushStyleColor(ImGuiCol_HeaderActive, IM_COL32(0, 0, 0, 0));  // button clicked
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
  ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);

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
  const auto p_tl = ImGui::GetCursorScreenPos();
  const auto p_br = p_tl + size;
  const auto p_wh = size;

  // draw_list->ChannelsSplit(2);
  // draw_list->ChannelsSetCurrent(1);

  const ImVec2 mouse_delta = ImGui::GetIO().MouseDelta;
  const bool mouse_move = mouse_delta.x != 0.0f || mouse_delta.y != 0.0f;

  const std::string id = def.imgui_hash;
  ImGui::Selectable(id.c_str(), false, 0, size);
  const bool is_hovered = ImGui::IsItemHovered();
  const bool is_clicked = ImGui::IsItemClicked();

  bool newly_selected = false;
  auto old_active_cell = def.active_cell;

  if (is_hovered && mouse_move && def.update_selected_on_mouse_move)
    def.active_cell = def.cell;

  if (is_clicked) {
    def.active_cell = def.cell;
    do_act = true;
  }

  if (def.active_cell != old_active_cell)
    newly_selected = true;

  bool is_selected = def.active_cell == def.cell;
  if (def.update_selected_only_with_mouse)
    is_selected = is_hovered;

  // draw_list->ChannelsSetCurrent(0);
  // const auto p_tl = ImGui::GetItemRectMin();
  // const auto p_br = ImGui::GetItemRectMax();
  // const auto p_wh = ImGui::GetItemRectSize();

  const ImU32 im_inactive_outline_col = convert_my_to_im(def.inactive_outline_col);
  const ImU32 im_active_outline_col = convert_my_to_im(def.active_outline_col);
  const ImU32 im_inactive_bg_col = convert_my_to_im(def.inactive_bg_col);
  const ImU32 im_active_bg_col = convert_my_to_im(def.active_bg_col);

  const ImU32 outline_col = is_selected ? im_active_outline_col : im_inactive_outline_col;
  const ImU32 bg_col = is_selected ? im_active_bg_col : im_inactive_bg_col;
  const ImU32 text_col = IM_COL32(255, 255, 255, is_selected ? 255 : 150);
  const ImVec4 text_col_vec = ImVec4(1.0f, 1.0f, 1.0f, is_selected ? 1.0f : 1.0f * (150 / 255.0f));

  // button
  draw_list->AddRectFilled(p_tl, p_br, bg_col, def.rounding);
  draw_list->AddRect(p_tl, p_br, outline_col, def.rounding, ImDrawFlags_RoundCornersAll, def.thickness);

  auto* font = def.font;
  if (font == nullptr)
    font = ImGui::GetIO().Fonts->Fonts[0];

  const auto label = def.display_str;

  ImGui::PushFont(font, def.font_size);
  const auto text_size = ImGui::CalcTextSize(label.c_str());
  auto text_pos = ImVec2{ p_tl.x, p_tl.y };
  text_pos.x += def.text_pivot.x * (p_wh.x - text_size.x);
  text_pos.y += def.text_pivot.y * (p_wh.y - text_size.y);
  text_pos += def.text_offset;
  draw_list->AddText(ImGui::GetFont(), ImGui::GetFontSize(), text_pos, text_col, label.c_str());
  ImGui::PopFont();

  // draw as an icon
  if (def.icon.has_value()) {
    const auto& icon = def.icon.value();
    const auto space_x = p_br.x - p_tl.x;
    const auto space_y = p_br.y - p_tl.y;
    const auto pos_x = p_tl.x + 0.5f * (space_x - def.icon_size.x);
    const auto pos_y = p_tl.y + 0.5f * (space_y - def.icon_size.y);
    const auto icon_p_tl = ImVec2{ pos_x, pos_y };
    const auto icon_p_br = ImVec2{ pos_x + def.icon_size.x, pos_y + def.icon_size.y };
    const auto tex_id = search_for_texture_id_by_texture_path(ri_c, "custom.png(GL_NEAREST)")->id;
    const auto im_id = (ImTextureID)(void*)(intptr_t)tex_id;
    const auto [icon_uv_tl, icon_uv_br] = convert_sprite_to_uv(r, icon);
    draw_list->AddImage(im_id, icon_p_tl, icon_p_br, icon_uv_tl, icon_uv_br, text_col);
  }

  // "commit changes"
  // draw_list->ChannelsMerge();

  // input
  if (def.input && is_selected)
    do_act = true;

  // Do the callback for the button
  if (is_selected && do_act && def.play_audio)
    create_empty<AudioRequestPlayEvent>(r, AudioRequestPlayEvent{ "UI_SELECT_0" });
  if (newly_selected && def.play_audio)
    create_empty<AudioRequestPlayEvent>(r, AudioRequestPlayEvent{ "UI_HOVER_0" });

  ImGui::PopStyleColor(2);
  ImGui::PopStyleVar(4);

  return do_act;
};

void
process_input_for_ui_all_handles(entt::registry& r, UIState& state)
{
  // GET_FIRST_OR_RETURN(SINGLE_SteamConnectedControllers, r, steam_con_e, steam_con_c)

  state.actions.clear();

  const auto input_e = get_first<InputComponent, Persistent>(r);
  const auto& input_c = r.get<InputComponent>(input_e);
  process_input_for_ui(r, state, input_c);
};

void
process_input_for_ui(entt::registry& r, UIState& state, const InputComponent& inp_c)
{
  if (state.active == nullptr) {
    // SDL_Log("state.active is null. You went back to the main menu.");
    return;
  }

  if (has(inp_c.dpad_u, ActionStateEnum::DOWN)) {
    if (state.active->u != nullptr)
      state.active = state.active->u;
    state.actions.push_back(UIAction::NAV_MOVE_U);
  }
  if (has(inp_c.dpad_d, ActionStateEnum::DOWN)) {
    if (state.active->d != nullptr)
      state.active = state.active->d;
    state.actions.push_back(UIAction::NAV_MOVE_D);
  }
  if (has(inp_c.dpad_l, ActionStateEnum::DOWN)) {
    if (state.active->l != nullptr)
      state.active = state.active->l;
    state.actions.push_back(UIAction::NAV_MOVE_L);
  }
  if (has(inp_c.dpad_r, ActionStateEnum::DOWN)) {
    if (state.active->r != nullptr)
      state.active = state.active->r;
    state.actions.push_back(UIAction::NAV_MOVE_R);
  }

  if (has(inp_c.button_s, ActionStateEnum::DOWN))
    state.actions.push_back(UIAction::SELECT);

  if (has(inp_c.button_e, ActionStateEnum::DOWN))
    state.actions.push_back(UIAction::BACK);
};

void
clear_navlinks(std::vector<std::shared_ptr<Cell>>& cells)
{
  for (int i = 0; i < cells.size(); i++) {
    auto& c = cells[i];
    c->u = nullptr;
    c->d = nullptr;
    c->l = nullptr;
    c->r = nullptr;
  }
}

void
create_as_vertical_layout(std::vector<std::shared_ptr<Cell>>& cells)
{
  //
  // sort out navigations
  // let every button reference the buttons above/below
  // this is basically a vertical layout group
  //
  for (int i = 0; i < cells.size(); i++) {
    const auto last = engine::wrap(i - 1, (int)cells.size());
    const auto next = engine::wrap(i + 1, (int)cells.size());
    const auto& prv = cells[last];
    const auto& nxt = cells[next];
    auto& c = cells[i];
    c->u = prv;
    c->d = nxt;
  }
}

void
create_as_horizontal_layout(std::vector<std::shared_ptr<Cell>>& cells)
{
  for (int i = 0; i < cells.size(); i++) {
    const auto last = engine::wrap(i - 1, (int)cells.size());
    const auto next = engine::wrap(i + 1, (int)cells.size());
    const auto& prv = cells[last];
    const auto& nxt = cells[next];
    auto& c = cells[i];
    c->l = prv;
    c->r = nxt;
  }
}

} // namespace game2d