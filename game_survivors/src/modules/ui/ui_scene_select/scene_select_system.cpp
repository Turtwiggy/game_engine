#include "pch.hpp"

#include "scene_select_components.hpp"
#include "scene_select_helpers.hpp"
#include "scene_select_system.hpp"

#include "engine/colour/colour.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/events/components.hpp"
#include "engine/imgui/helpers.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/actors/actor_weapon/weapon_components.hpp"
#include "modules/core/fonts/fonts_helpers.hpp"
#include "modules/core/renderer/components.hpp"
#include "modules/core/renderer/helpers.hpp"
#include "modules/core/ui/ui_common_components.hpp"
#include "modules/core/ui/ui_common_helpers.hpp"
#include "modules/scene/scene_components.hpp"
#include "modules/scene/scene_helpers.hpp"
#include "modules/steam_input/steam_input_components.hpp"
#include "modules/steam_input/steam_input_helpers.hpp"
#include "modules/systems/system_hardpoint_arcs/hulls_components.hpp"
#include "modules/ui/ui_colours/ui_colours_helpers.hpp"
#include "modules/ui/ui_popup_options/ui_popup_options_components.hpp"
#include "modules/ui/ui_scene_main_menu_controllerinfo/ui_main_menu_controllerinfo_components.hpp"
#include "modules/ui/ui_scene_main_menu_controllerinfo/ui_main_menu_controllerinfo_helpers.hpp"
#include "resources/data.hpp"

namespace game2d {

const auto my_bg_col = hex_to_srgb("#0c1116");

std::string
str_remove_all_occurances(std::string base, const std::string& substr)
{
  size_t pos = 0;
  while ((pos = base.find(substr, pos)) != std::string::npos)
    base.erase(pos, substr.length());
  return base;
};

struct TextDesc
{
  std::string text = "";
  ImVec2 non_centered_pos{ 0, 0 };
  ImU32 col = 0;
  float wrap_width = -1;
  FontSize font_size = FontSize::TEXT_SIZE_16;
  FontSize font_size_scaled = FontSize::TEXT_SIZE_16_SCALED;
};

void
add_text_centered_here(entt::registry& r, ImDrawList* draw_list, const TextDesc& desc, ImVec2* out_pos = nullptr)
{
  const auto font_scale = get_first_component<SINGLE_UIScaling>(r).scaling;
  const auto font_enum = font_scale == 1.0f ? desc.font_size : desc.font_size_scaled;
  const auto font_size = (float)font_enum;
  auto* font = get_inter_font(r, font_enum);

  const auto text_size = font->CalcTextSizeA(font_size, FLT_MAX, desc.wrap_width, desc.text.c_str());
  const auto text_pos = desc.non_centered_pos - ImVec2{ 0.5f * text_size.x, 0.5f * text_size.y };
  draw_list->AddText(font, font_size, text_pos, desc.col, desc.text.c_str(), NULL, desc.wrap_width);

  if (out_pos) {
    out_pos->x = text_pos.x;
    out_pos->y = text_pos.y;
  }
};

/*

void
update_input_for_confirm(entt::registry& r, const int player_idx)
{
  GET_FIRST_OR_RETURN(SINGLE_SelectSceneData, r, ui_e, ui_c)
  GET_FIRST_OR_RETURN(SINGLE_Hulls, r, hulls_e, hulls_c)
  GET_FIRST_OR_RETURN(SINGLE_Weapons, r, weapons_e, weapons_c)

  auto& player_ui_data = ui_c.player_ui_state[player_idx].state;
  auto& player_state = ui_c.player_choice_state[player_idx];

  const auto& a = player_ui_data.actions;
  const bool player_pressed_select = std::find(a.begin(), a.end(), UIAction::SELECT) != a.end();
  const bool player_pressed_back = std::find(a.begin(), a.end(), UIAction::BACK) != a.end();
  const bool v_value_changed_u = std::find(a.begin(), a.end(), UIAction::NAV_MOVE_U) != a.end();
  const bool v_value_changed_d = std::find(a.begin(), a.end(), UIAction::NAV_MOVE_D) != a.end();
  const bool h_value_changed_l = std::find(a.begin(), a.end(), UIAction::NAV_MOVE_L) != a.end();
  const bool h_value_changed_r = std::find(a.begin(), a.end(), UIAction::NAV_MOVE_R) != a.end();
  const bool v_value_changed = v_value_changed_u || v_value_changed_d;
  const bool h_value_changed = h_value_changed_l || h_value_changed_r;

  if (player_pressed_select) {

    // ready if you're on the last row
    const bool ready = player_state.player_row_idx == (player_ui_data.cells.size() - 1);

    player_state.confirmed = ready;
    if (ready)
      SDL_Log("A player is ready");

    player_state.player_row_idx++;
    player_state.player_row_idx = glm::clamp(player_state.player_row_idx, 0, (int)(player_ui_data.cells.size() - 1));
  }

  if (player_pressed_back) {
    player_state.confirmed = false;

    // IDEA: could make the player hold button for menu transition
    const bool back_to_menu = player_state.player_row_idx == 0;
    if (back_to_menu) {
      move_to_scene_start(r, Scene::menu);
      return;
    }

    player_state.player_row_idx--;
    player_state.player_row_idx = glm::clamp(player_state.player_row_idx, 0, (int)(player_ui_data.cells.size() - 1));
  }

  // player_ui_data.current_row_index = player_state.player_row_idx;

  // Clamp the values.
  {
    auto* ui_hulls = dynamic_cast<OptionsCell*>(player_ui_data.cells[0].get());
    const auto& hulls = hulls_c.hulls;
    auto& idx = ui_hulls->value;
    idx = idx < 0 ? (int)hulls.size() - 1 : idx;
    idx %= hulls.size();
  }
  {
    auto* ui_weapons = dynamic_cast<OptionsCell*>(player_ui_data.cells[1].get());
    const auto& weapons = weapons_c.weapons;
    auto& idx = ui_weapons->value;
    idx = idx < 0 ? (int)weapons.size() - 1 : idx;
    idx %= weapons.size();
  }
};

void
draw_main_header_quarters(entt::registry& r, const ImVec2 tl, const ImVec2 wh, const int player_idx)
{
  GET_FIRST_OR_RETURN(SINGLE_SelectSceneData, r, ui_e, ui_c)
  const auto& player_ui_data = ui_c.player_ui_state[player_idx];
  const auto& player_state = ui_c.player_choice_state[player_idx];

  auto* draw_list = ImGui::GetWindowDrawList();
  // draw a background
  // const auto br = ImVec2{ tl.x + wh.x, tl.y + wh.y };
  // draw_list->AddRectFilled(tl, br, im_active_col, 6);

  // split the box in to thirds.
  const float header_segments = 2;
  auto box_tl = ImVec2(tl.x, tl.y);
  auto box_wh = ImVec2(wh.x * (1.0f / header_segments), wh.y);
  for (int i = 0; i < header_segments; i++) {

    const bool is_hull = i == 0;
    const bool is_weapon = i == 1;
    const bool is_ability = i == 2;
    const bool active = i == player_state.player_row_idx;
    const int active_alpha = active ? 255 : 100;

    const auto& base = player_ui_data.state.cells[i];
    const auto* cell = dynamic_cast<OptionsCell*>(base.get());
    const auto col_idx = cell->value;

    const auto my_player_col = default_player_colours[player_idx];
    const auto im_player_col = IM_COL32(my_player_col.r, my_player_col.g, my_player_col.b, active_alpha);
    const auto im_bg_col = IM_COL32(my_bg_col.r, my_bg_col.g, my_bg_col.b, active_alpha);
    const auto rounding = 4.0f;

    // draw a rect with filled border
    const auto box_br = ImVec2(box_tl.x + box_wh.x, box_tl.y + box_wh.y);
    draw_list->AddRectFilled(box_tl, box_br, im_bg_col, 8);
    draw_list->AddRect(box_tl, box_br, im_player_col, rounding, 0, 2.0);

    const auto box_center = ImVec2(box_tl.x + 0.5f * box_wh.x, box_tl.y + 0.5f * box_wh.y);

    // Hulls info
    if (is_hull) {
      const auto draw_hulls_header = [&]() {
        const auto& hulls_c = get_first_component<SINGLE_Hulls>(r);
        const auto text_str = std::format("Hull {}/{}", col_idx + 1, hulls_c.hulls.size());
        // const auto text_str = std::format("Hull");

        const TextDesc text_desc{
          .text = text_str,
          .non_centered_pos = box_center,
          .col = IM_COL32(255, 255, 255, active_alpha),
          .wrap_width = -1,
        };
        add_text_centered_here(r, draw_list, text_desc);
      };
      draw_hulls_header();
    }

    // Weapons info
    if (is_weapon) {
      const auto draw_weapons_header = [&]() {
        const auto& weapons_c = get_first_component<SINGLE_Weapons>(r);
        const auto text_str = std::format("Weapon {}/{}", col_idx + 1, weapons_c.weapons.size());
        // const auto text_str = std::format("Weapon");

        const TextDesc text_desc{
          .text = text_str,
          .non_centered_pos = box_center,
          .col = IM_COL32(255, 255, 255, active_alpha),
          .wrap_width = -1,
        };
        add_text_centered_here(r, draw_list, text_desc);
      };
      draw_weapons_header();
    }

    // Ability info
    /*
    if (is_ability) {
      const auto draw_ability_header = [&]() {
        const auto text_str = std::format("Ability {}/{}", 0, 0);

        const TextDesc text_desc{
          .text = "Ability",
          .non_centered_pos = box_center,
          .col = IM_COL32(255, 255, 255, active_alpha),
          .wrap_width = -1,
        };
        add_text_centered_here(r, draw_list, text_desc);
      };
      draw_ability_header();
    }

// move the segments on.
box_tl.x += box_wh.x;
}
}
;

void
draw_main_quarters(entt::registry& r, const ImVec2 tl, const ImVec2 wh, const int player_idx)
{
  GET_FIRST_OR_RETURN(SINGLE_SelectSceneData, r, ui_e, ui_c)
  GET_FIRST_OR_RETURN(SINGLE_Hulls, r, hulls_e, hulls_c)
  GET_FIRST_OR_RETURN(SINGLE_Weapons, r, weapons_e, weapons_c)
  GET_FIRST_OR_RETURN(SINGLE_RendererInfo, r, ri_e, ri_c)
  const auto tex_id = search_for_texture_id_by_texture_path(ri_c, "monochrome")->id;
  const auto im_id = reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(tex_id));
  const auto ui_scaling = get_first_component<SINGLE_UIScaling>(r).scaling;
  const auto& ui_state = ui_c.player_ui_state[player_idx].state;

  // draw a background
  const auto im_bg_col = IM_COL32(my_bg_col.r, my_bg_col.g, my_bg_col.b, 255);
  const auto my_player_col = default_player_colours[player_idx];
  const auto im_player_col = convert_my_to_im(my_player_col);
  const auto br = ImVec2{ tl.x + wh.x, tl.y + wh.y };
  auto* draw_list = ImGui::GetWindowDrawList();
  draw_list->AddRectFilled(tl, br, im_bg_col, 8);
  // draw_list->AddRect(tl, br, im_player_col, 8, 0, 2.0);

  // std::string& hull = player_state.player_boat;
  // std::string& weapon = player_state.player_gun;
  // std::string& ability = player_state.player_ability;

  // split the box in to thirds horizontally
  const float segments = 2; // change to 3 for ability header
  auto box_tl = ImVec2(tl.x, tl.y);
  auto box_wh = ImVec2(wh.x * (1.0f / segments), wh.y);
  for (int i = 0; i < segments; i++) {
    // data
    const bool is_hull = i == 0;
    const bool is_weapon = i == 1;
    const bool is_ability = i == 2;
    const bool active = ui_state.cells[i] == ui_state.active;
    const int active_alpha = active ? 255 : 100;
    const int col_idx = dynamic_cast<OptionsCell*>(ui_state.cells[i].get())->value;

    // draw a debug rect
    // const float seg_inc = ((player_idx + 1) / segments);
    // const auto seg_im_active_col = IM_COL32(255 * seg_inc, 0, 0, 255);
    // const auto box_br = ImVec2(box_tl.x + box_wh.x, box_tl.y + box_wh.y);
    // draw_list->AddRectFilled(box_tl, box_br, seg_im_active_col, 6);
    // std::string debug_str = std::format("ri {} ci {}", i, col_idx);
    // add_text_centered_here(ImGui::GetWindowDrawList(), debug_str, box_center, active_alpha);

    std::string name = "";
    std::string desc = "";

    // display hull
    if (is_hull) {
      const auto& hull = hulls_c.hulls[col_idx];
      auto& player_state = ui_c.player_choice_state[player_idx];
      player_state.player_boat_key = hull.key;

      // split the name across lines
      auto str = hull.name;
      std::replace(str.begin(), str.end(), ' ', '\n');
      name = str;
      desc = hull.desc;
    }

    // display weapon
    if (is_weapon) {
      const auto& idx = col_idx;
      const auto& weapon = weapons_c.weapons[idx];
      auto& player_state = ui_c.player_choice_state[player_idx];
      player_state.player_gun_key = weapon.key;

      // split the name across lines
      auto str = weapon.name;
      std::replace(str.begin(), str.end(), ' ', '\n');
      name = str;
      desc = weapon.desc;
    }

    if (is_ability) {
      name = "N/A";
      desc = "None";
    }

    const auto my_desc_text_col = engine::SRGBColour(200, 200, 200, active_alpha);
    const auto im_desc_text_col = convert_my_to_im(my_desc_text_col);

    // const auto head_font_enum = font_scale == 1.0f ? FontSize::TEXT_SIZE_16 : FontSize::TEXT_SIZE_16_SCALED;
    // const auto box_center = ImVec2(box_tl.x + 0.5f * box_wh.x, box_tl.y + 0.5f * box_wh.y);

    const auto header_y = box_tl.y + 0.15f * box_wh.y;
    const auto header_padding = 4;
    const auto header_pos = ImVec2(box_tl.x + 0.5f * box_wh.x + header_padding, header_y);
    const TextDesc header_text_desc{
      .text = name,
      .non_centered_pos = header_pos,
      .col = IM_COL32(255, 255, 255, active_alpha),
      .wrap_width = -1,
      .font_size = FontSize::TEXT_SIZE_16,
      .font_size_scaled = FontSize::TEXT_SIZE_16_SCALED,
    };
    ImVec2 header_text_pos_centered{ 0, 0 };
    add_text_centered_here(r, draw_list, header_text_desc, &header_text_pos_centered);

    // draw description text
    {
      const auto font_scale = get_first_component<SINGLE_UIScaling>(r).scaling;
      const auto font_enum = font_scale == 1.0f ? FontSize::TEXT_SIZE_13 : FontSize::TEXT_SIZE_13_SCALED;
      const auto font_size = (float)font_enum;
      const auto* font = get_inter_font(r, font_enum);

      const float desc_pad_x = 4;
      const float width_limit = box_wh.x - (2.0f * desc_pad_x);
      const auto text_size = font->CalcTextSizeA(font_size, width_limit, width_limit, desc.c_str());

      const auto desc_text_pos = ImVec2(box_tl.x + 0.5f * box_wh.x + desc_pad_x, box_tl.y + 0.3f * box_wh.y);
      const auto desc_text_pos_centered = ImVec2(desc_text_pos.x - 0.5f * text_size.x, desc_text_pos.y);
      draw_list->AddText(font, font_size, desc_text_pos_centered, im_desc_text_col, desc.c_str(), NULL, width_limit);
    }

    // draw some left and right arrows
    if (active) {
      const auto font_scale = get_first_component<SINGLE_UIScaling>(r).scaling;
      const auto font_enum = font_scale == 1.0f ? FontSize::TEXT_SMALL : FontSize::TEXT_SMALL_SCALED;
      const auto font_size = (float)font_enum;
      auto* font = get_inter_font(r, font_enum);

      const std::string arrow_l = "<";
      const std::string arrow_r = ">";
      const auto l_text_size = font->CalcTextSizeA(font_size, FLT_MAX, -1.0f, arrow_l.c_str());
      const auto r_text_size = font->CalcTextSizeA(font_size, FLT_MAX, -1.0f, arrow_r.c_str());

      const float padding_x = 8; // pad the arrows away from the edge
      const auto l = ImVec2{ box_tl.x + padding_x, header_y - 0.5f * l_text_size.y };
      const auto r = ImVec2{ box_tl.x + box_wh.x - r_text_size.x - padding_x, header_y - 0.5f * r_text_size.y };

      const auto l_text_pos = l;
      const auto r_text_pos = r;
      draw_list->AddText(font, font_size, l_text_pos, IM_COL32(150, 150, 150, 255), arrow_l.c_str());
      draw_list->AddText(font, font_size, r_text_pos, IM_COL32(150, 150, 150, 255), arrow_r.c_str());
    }

    // move the segments on.
    box_tl.x += box_wh.x;
  }
};

struct DisplayStat
{
  std::string key;
  std::string val;
};

void
draw_selected_info_panel(entt::registry& r, const ImVec2 tl, const ImVec2 wh, const int player_idx)
{
  GET_FIRST_OR_RETURN(SINGLE_SelectSceneData, r, ui_e, ui_c)
  GET_FIRST_OR_RETURN(SINGLE_Hulls, r, hulls_e, hulls_c)
  GET_FIRST_OR_RETURN(SINGLE_Weapons, r, weapons_e, weapons_c)
  const auto& player_ui_data = ui_c.player_ui_state[player_idx];
  const auto& player_state = ui_c.player_choice_state[player_idx];
  const auto ui_scaling = get_first_component<SINGLE_UIScaling>(r).scaling;

  const auto player_row_idx = player_ui_data.current_row_index;
  const bool is_hull = player_ui_data.current_row_index == 0;
  const bool is_weapon = player_ui_data.current_row_index == 1;
  const bool is_ability = player_ui_data.current_row_index == 2;
  int col_idx = player_ui_data.rows[player_ui_data.current_row_index].col_index;

  std::string header_txt = "";
  std::string description_txt = "";
  std::string more_info_txt = "";

  std::vector<DisplayStat> display_stats;

  if (is_hull) {
    const auto& hull = hulls_c.hulls[col_idx];
    header_txt = hull.name;
    description_txt = hull.desc;
    display_stats.push_back({ .key = "Hardpoints", .val = std::to_string(hull.hardpoints.size()) });
    display_stats.push_back({ .key = "Size", .val = std::to_string(hull.width * hull.height) });
  }

  if (is_weapon) {
    const auto& weapon = weapons_c.weapons[col_idx];
    header_txt = weapon.name;
    description_txt = weapon.desc;
    for (const auto& [key, val] : weapon.data) {
      auto clean_key = key;
      clean_key = str_remove_all_occurances(clean_key, "WEAPON_");
      clean_key = str_remove_all_occurances(clean_key, "BULLET_");
      display_stats.push_back({ .key = clean_key, .val = std::format("{:.2f}", val) });
    }
  }

  const auto font_scale = get_first_component<SINGLE_UIScaling>(r).scaling;
  const auto font_enum = font_scale == 1.0f ? FontSize::TEXT_SIZE_16 : FontSize::TEXT_SIZE_16_SCALED;
  auto* font = get_inter_font(r, font_enum);

  // use h not wh.y
  static float space_per_row = (float)font_enum;
#if defined(_DEBUG)
  imgui_draw_float("space_per_row", space_per_row);
#endif
  const float h = (display_stats.size() + 1) * space_per_row; // +1, row for header

  // draw a background
  const auto my_bg_col = default_player_colours[player_idx];
  const auto im_bg_col = IM_COL32(my_bg_col.r, my_bg_col.g, my_bg_col.b, 0.1f * 255);
  const auto im_border_col = IM_COL32(my_bg_col.r, my_bg_col.g, my_bg_col.b, 0.2f * 255);
  const auto br = ImVec2{ tl.x + wh.x, tl.y + h };
  auto* draw_list = ImGui::GetWindowDrawList();
  draw_list->AddRectFilled(tl, br, im_bg_col, 8);
  draw_list->AddRect(tl, br, im_border_col, 8, 0, 2.0f);

  // Draw header + description
  const std::string header_text = header_txt;
  auto header_pos = ImVec2(tl.x + 0.5f * wh.x, tl.y + 0.5f * space_per_row);
  auto header_size = ImGui::CalcTextSize(header_text.c_str());
  header_pos -= { 0.5f * header_size.x, 0.5f * header_size.y };
  draw_list->AddText(header_pos, IM_COL32(255, 255, 255, 255), header_text.c_str());

  auto box_tl = ImVec2(tl.x, tl.y + (1.0f * space_per_row)); // offset by 1 row
  auto box_wh = ImVec2(wh.x, h);

  for (int idx = 0; idx < (int)display_stats.size(); idx++) {
    const auto stat = display_stats[idx];

    // draw stat
    const auto stat_head_pos = ImVec2(box_tl.x + 0.1f * box_wh.x, box_tl.y);
    const auto stat_desc_pos = ImVec2(box_tl.x + 0.66f * box_wh.x, box_tl.y);
    draw_list->AddText(stat_head_pos, IM_COL32(172, 172, 172, 255), stat.key.c_str());
    draw_list->AddText(stat_desc_pos, IM_COL32(172, 172, 172, 255), stat.val.c_str());

    // move on padding.
    box_tl.y += space_per_row;
  }
}
*/

void
update_split_screen_into_quarters(entt::registry& r,
                                  SINGLE_RendererInfo& ri_c,
                                  SINGLE_SelectSceneData& ui_c,
                                  const int max_num_players)
{
  GET_FIRST_OR_RETURN(SINGLE_SteamControllerGameState, r, steam_ui_e, steam_ui_c);
  GET_FIRST_OR_RETURN(SINGLE_SteamControllers, r, steam_e, steam_c);
  const auto ui_scaling = get_first_component<SINGLE_UIScaling>(r).scaling;

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoDecoration;
  flags |= ImGuiWindowFlags_NoMove;
  flags |= ImGuiWindowFlags_NoCollapse;
  flags |= ImGuiWindowFlags_NoDocking;
  flags |= ImGuiWindowFlags_NoBackground;
  flags |= ImGuiWindowFlags_NoInputs;
  flags |= ImGuiWindowFlags_NoSavedSettings;

  const auto set_window_pos = ImVec2{ ri_c.viewport_size_render_at.x * 0.5f, ri_c.viewport_size_render_at.y * 0.5f };
  const auto set_window_size = ImVec2{
    (float)ri_c.viewport_size_render_at.x,
    (float)ri_c.viewport_size_render_at.y,
  };
  ImGui::SetNextWindowPos(set_window_pos, ImGuiCond_Always, { 0.5f, 0.5f });
  ImGui::SetNextWindowSize(set_window_size);

  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

  ImGui::Begin("SelectShipUI", nullptr, flags);

  const ImVec2 window_tl = ImGui::GetWindowPos();
  const ImVec2 window_wh = ImGui::GetWindowSize();
  const ImVec2 player_wh = { window_wh.x / max_num_players, window_wh.y };
  const auto num_active_players = glm::max(1, (int)steam_c.n_active);

  // center it.
  const auto center_x = window_tl.x + 0.5f * window_wh.x;
  const auto center_y = player_wh.y * 0.5f;
  auto first_tl_x = center_x;
  first_tl_x -= num_active_players * (0.5f * player_wh.x);

  for (int player_idx = 0; player_idx < max_num_players; player_idx++) {

    const auto handle = steam_ui_c.handles[player_idx];
    const bool connected = handle_is_connected(steam_c, handle);
    const bool joined = handle_is_joined(steam_ui_c, handle);

    // always show one player. player_idx either using keyboard or controller
    if ((connected && joined) || player_idx == 0) {

      // const auto width = 300;
      // const auto height = 120 * ui_scaling; // or 1/6th of the screen
      // const auto main_quarter_tl = ImVec2{ first_tl_x, center_y - (height * 0.5f) };
      // draw_main_quarters(r, main_quarter_tl, ImVec2{ player_wh.x, height }, player_idx);

      // const auto header_height = 25 * ui_scaling;
      // const auto header_tl = ImVec2(first_tl_x, main_quarter_tl.y - header_height);
      // draw_main_header_quarters(r, header_tl, { player_wh.x, header_height }, player_idx);

      // const auto selected_info_w = player_wh.x * 0.6f; // X% of the width of the quarter.
      // const auto selected_info_y = player_wh.y * 0.667f;
      // const auto selected_info_tl = ImVec2(first_tl_x + (0.5f * player_wh.x) - (0.5f * selected_info_w), selected_info_y);
      // draw_selected_info_panel(r, selected_info_tl, { selected_info_w, 0 }, player_idx);
    }

    else
      break; // no more players

    // move horizontally
    first_tl_x += player_wh.x;
  }

  ImGui::End();
  ImGui::PopStyleVar(2);
};

void
update_ui_scene_select_system(entt::registry& r, const float dt)
{
  GET_FIRST_OR_RETURN(SINGLE_SelectSceneData, r, ui_e, ui_c)
  GET_FIRST_OR_RETURN(SINGLE_RendererInfo, r, ri_e, ri_c);
  GET_FIRST_OR_RETURN(SINGLE_InputComponent, r, input_e, input_c)
  GET_FIRST_OR_RETURN(SINGLE_Hulls, r, hulls_e, hulls_c)
  GET_FIRST_OR_RETURN(SINGLE_Weapons, r, weapons_e, weapons_c)
  GET_FIRST_OR_RETURN(SINGLE_SteamControllerGameState, r, steam_state_e, steam_state_c)
  GET_FIRST_OR_RETURN(SINGLE_SteamControllers, r, steam_e, steam_c)

  const int max_num_players = 4;
  const auto num_active_players = glm::max(1, (int)steam_c.n_active);

  if (ui_c.menu_to_select_scene_buffer_frame) {
    ui_c.menu_to_select_scene_buffer_frame = false;
    ui_c.player_ui_state.resize(max_num_players);
    ui_c.player_choice_state.resize(max_num_players);
    return;
  }

  // controllers 1-4 works for ui 1-4
  update_input_for_select_ui(r, ui_c);

  // for (int i = 0; i < max_num_players; i++)
  //   update_input_for_confirm(r, i);

  update_split_screen_into_quarters(r, ri_c, ui_c, max_num_players);

  const auto& scene_c = get_first_component<SINGLE_CurrentScene>(r);
  if (scene_c.s != Scene::select_ships)
    return;

  //
  // Show a countdown timer when all joined players are ready
  //

  const std::vector<HullChoice> c = { ui_c.player_choice_state.begin(),
                                      ui_c.player_choice_state.begin() + num_active_players };

  const auto confirmed = [](const HullChoice& choice) { return choice.confirmed; };
  auto everyone_confirmed = std::all_of(c.begin(), c.end(), confirmed);
  auto someone_confirmed = std::any_of(c.begin(), c.end(), confirmed);
  auto noone_confirmed = std::none_of(c.begin(), c.end(), confirmed);
  everyone_confirmed &= c.size() > 0;
  someone_confirmed &= c.size() > 0;
  noone_confirmed &= c.size() == 0;

  ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2{ 0.5f, 0.5f });
  update_countdown(ui_c, everyone_confirmed, dt);
  if (everyone_confirmed)
    update_countdown_ui(r, ui_c);
  update_countdown_to_next_scene(r, ui_c, hulls_c.hulls);

  ImGui::PopStyleVar();
}

} // namespace game2d
