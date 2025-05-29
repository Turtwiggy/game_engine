#include "pch.hpp"

#include "scene_select_components.hpp"
#include "scene_select_helpers.hpp"
#include "scene_select_system.hpp"

#include "engine/colour/colour.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/events/components.hpp"
#include "engine/imgui/helpers.hpp"
#include "engine/imgui/ui_imgui_defaults.hpp"
#include "engine/maths/maths.hpp"
#include "engine/physics/physics_helpers.hpp"
#include "engine/std/string/helpers.hpp"
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

struct TextDesc
{
  std::string text = "";
  ImVec2 non_centered_pos{ 0, 0 };
  ImU32 col = 0;
  float wrap_width = -1;
  FontSize font_size = FontSize::TEXT_SIZE_16;
  FontSize font_size_scaled = FontSize::TEXT_SIZE_16_SCALED;
};

struct DisplayStat
{
  std::string key;
  std::string val;
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

void
draw_selections(entt::registry& r,
                SINGLE_SelectSceneData& ui_c,
                const std::shared_ptr<Cell>& base,
                int player_idx,
                int cell_idx,
                const ImVec2 pos,
                const ImVec2 button_size,
                const bool active)
{
  GET_FIRST_OR_RETURN(SINGLE_Hulls, r, hulls_e, hulls_c)
  GET_FIRST_OR_RETURN(SINGLE_Weapons, r, weapons_e, weapons_c)
  auto& player_state = ui_c.player_choice_state[player_idx];

  auto* cell = dynamic_cast<OptionsCell*>(base.get());
  const bool is_hull = cell_idx == 0;
  const bool is_weapon = cell_idx == 1;
  const bool is_ability = cell_idx == 2;

  std::string name = "";

  // convert index to hull choice
  if (is_hull) {
    cell->value = engine::wrap(cell->value, hulls_c.hulls.size() - 1);
    const auto& hull = hulls_c.hulls[cell->value];
    name = hull.name;
    player_state.player_boat_key = hull.key;
  }

  // convert index to weapon choice
  if (is_weapon) {
    cell->value = engine::wrap(cell->value, weapons_c.weapons.size() - 1);
    const auto& weapon = weapons_c.weapons[cell->value];
    name = weapon.name.c_str();
    player_state.player_gun_key = weapon.key;
  }

  // convert index to ability choice.
  if (is_ability)
    name = "None";

  const auto text = active ? std::format("< {} >", name) : name;
  const auto text_wh = ImGui::CalcTextSize(text.c_str());

  // center the text y
  ImGui::SetCursorPos({ pos.x - 0.5f * text_wh.x, pos.y + 0.5f * (button_size.y - text_wh.y) });

  if (active) {
    ImGui::Text("< ");
    ImGui::SameLine();
    ImGui::TextColored(im_text_col, "%s", name.c_str());
    ImGui::SameLine();
    ImGui::Text(" >");
  } else {
    ImGui::TextColored(im_text_disabled_col, "%s", name.c_str());
  }
};

void
draw_stats(entt::registry& r, ImVec2 box_tl, ImVec2 box_wh, SelectUI& player_ui_c)
{
  GET_FIRST_OR_RETURN(SINGLE_Hulls, r, hulls_e, hulls_c)
  GET_FIRST_OR_RETURN(SINGLE_Weapons, r, weapons_e, weapons_c)

  // const auto font_scale = get_first_component<SINGLE_UIScaling>(r).scaling;
  // const auto font_enum = font_scale == 1.0f ? FontSize::TEXT_SIZE_16 : FontSize::TEXT_SIZE_16_SCALED;
  // auto* font = get_inter_font(r, font_enum);

  const auto box_br = box_tl + box_wh;
  auto* draw_list = ImGui::GetWindowDrawList();

  // add a background
  // draw_list->AddRectFilled(box_tl, box_br, IM_COL32(255, 0, 0, 255), 6);

  const auto& active_cell = player_ui_c.state.active;
  const auto& cs = player_ui_c.state.cells;
  const auto cell_it = std::find(cs.begin(), cs.end(), active_cell);
  const auto cell_idx = static_cast<int>(cell_it - cs.begin());
  auto* cell = dynamic_cast<OptionsCell*>(cell_it->get());
  const bool is_hull = cell_idx == 0;
  const bool is_weapon = cell_idx == 1;
  const bool is_ability = cell_idx == 2;

  std::string info_key = "";
  std::string info_desc = "";

  if (is_hull) {
    const auto& hull = hulls_c.hulls[cell->value];
    info_key = hull.name;
    info_desc = hull.desc;
  }
  if (is_weapon) {
    const auto& weapon = weapons_c.weapons[cell->value];
    info_key = weapon.name;
    info_desc = weapon.desc;
  }
  if (is_ability) {
  }

  const auto head_pos = ImVec2(box_tl.x + 0.5f * box_wh.x, box_tl.y + 0.04f * box_wh.y);
  const auto desc_pos = ImVec2(box_tl.x + 0.5f * box_wh.x, box_tl.y + 0.09f * box_wh.y);
  const auto stat_pos = ImVec2(box_tl.x + 0.5f * box_wh.x, box_tl.y + 0.21f * box_wh.y);

  // header text.
  {
    const auto header = info_key;

    const TextDesc header_text_desc{
      .text = header,
      .non_centered_pos = head_pos,
      .col = IM_COL32(255, 255, 255, 255),
      .wrap_width = -1,
      .font_size = FontSize::TEXT_SIZE_16,
      .font_size_scaled = FontSize::TEXT_SIZE_16_SCALED,
    };
    ImVec2 header_text_pos_centered{ 0, 0 };
    add_text_centered_here(r, draw_list, header_text_desc, &header_text_pos_centered);
  }

  // description text.
  {
    const std::string desc = info_desc;

    const auto font_scale = get_first_component<SINGLE_UIScaling>(r).scaling;
    const auto font_enum = font_scale == 1.0f ? FontSize::TEXT_SIZE_13 : FontSize::TEXT_SIZE_13_SCALED;
    const auto font_size = (float)font_enum;
    const auto* font = get_inter_font(r, font_enum);

    const float width_limit = box_wh.x - (2.0f * 4);
    const auto text_size = font->CalcTextSizeA(font_size, width_limit, width_limit, desc.c_str());

    const auto desc_text_pos_centered = ImVec2(desc_pos.x - 0.5f * text_size.x, desc_pos.y);
    draw_list->AddText(font, font_size, desc_text_pos_centered, im_text_col_vec, desc.c_str(), NULL, width_limit);
  }

  // stats.
  {
    std::vector<DisplayStat> display_stats;

    if (is_hull) {
      const auto& hull = hulls_c.hulls[cell->value];
      display_stats.push_back({ .key = "Hardpoints", .val = std::to_string(hull.hardpoints.size()) });
      display_stats.push_back({ .key = "Width", .val = std::format("{:.1f}m", pixels_to_meters(hull.width * 10)) });
      display_stats.push_back({ .key = "Height", .val = std::format("{:.1f}m", pixels_to_meters(hull.height * 10)) });
    }

    if (is_weapon) {
      const auto& weapon = weapons_c.weapons[cell->value];
      for (const auto& [key, val] : weapon.data) {
        auto clean_key = key;
        clean_key = str_remove_all_occurances(clean_key, "WEAPON_");
        clean_key = str_remove_all_occurances(clean_key, "BULLET_");
        display_stats.push_back({ .key = clean_key, .val = std::format("{:.2f}", val) });
      }

      // hack: if you're a sea turret, you deploy other weapons.
      // show the other weapon stats.
      if (weapon.key == "weapon_sea_turret") {
        auto deployed_weapon = weapons_c.weapons[0];
        display_stats.push_back({ .key = "      [DEPLOYS]", .val = deployed_weapon.name });
        for (const auto& [key, val] : deployed_weapon.data) {
          auto clean_key = key;
          clean_key = str_remove_all_occurances(clean_key, "WEAPON_");
          clean_key = str_remove_all_occurances(clean_key, "BULLET_");
          display_stats.push_back({ .key = clean_key, .val = std::format("{:.2f}", val) });
        }
      }
    }

    const auto text_wh = ImGui::CalcTextSize("A");

    for (int idx = 0; idx < (int)display_stats.size(); idx++) {
      const auto& stat = display_stats[idx];

      auto l_stat_pos = stat_pos;
      auto r_stat_pos = stat_pos;
      l_stat_pos.x = box_tl.x + 0.1f * box_wh.x;
      r_stat_pos.x = box_tl.x + 0.60f * box_wh.x;
      l_stat_pos.y += text_wh.y * idx;
      r_stat_pos.y += text_wh.y * idx;

      ImGui::SetCursorPos(l_stat_pos);
      ImGui::Text("%s", stat.key.c_str());

      ImGui::SetCursorPos(r_stat_pos);
      ImGui::Text("%s", stat.val.c_str());
    }
  }
};

void
update_player_select_ui(entt::registry& r,
                        SINGLE_RendererInfo& ri_c,
                        SINGLE_SelectSceneData& ui_c,
                        const int num_active_players,
                        const int max_num_players)
{

  GET_FIRST_OR_RETURN(SINGLE_SteamControllerGameState, r, steam_ui_e, steam_ui_c);
  GET_FIRST_OR_RETURN(SINGLE_SteamControllers, r, steam_e, steam_c);
  const auto ui_scaling = get_first_component<SINGLE_UIScaling>(r).scaling;

  const auto font_enum = ui_scaling == 1.0f ? FontSize::TEXT_SIZE_13 : FontSize::TEXT_SIZE_13_SCALED;
  const auto font_size = (float)font_enum;
  auto* font = get_inter_font(r, font_enum);
  ImGui::PushFont(font);

  const auto set_window_pos = ImVec2{ ri_c.viewport_size_render_at.x * 0.5f, ri_c.viewport_size_render_at.y * 0.5f };
  const auto set_window_size = ImVec2{ (float)ri_c.viewport_size_render_at.x, (float)ri_c.viewport_size_render_at.y };
  ImGui::SetNextWindowPos(set_window_pos, ImGuiCond_Always, { 0.5f, 0.5f });
  ImGui::SetNextWindowSize(set_window_size);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

  static float x_pad = 10;
  // imgui_draw_float("x_pad", x_pad);

  imgui_begin("SelectShipUI", ImGuiWindowFlags_NoInputs);
  const ImVec2 window_tl = ImGui::GetWindowPos();
  const ImVec2 window_wh = ImGui::GetWindowSize();
  const ImVec2 player_wh = { window_wh.x / max_num_players, window_wh.y };
  // const auto num_active_players = glm::max(1, (int)steam_c.n_active);
  // const auto num_active_players = 4;

  // center it.
  const auto center_x = window_tl.x + 0.5f * window_wh.x;
  const auto center_y = player_wh.y * 0.5f;
  auto first_tl_x = center_x;
  first_tl_x -= num_active_players * (0.5f * player_wh.x);

  for (int player_idx = 0; player_idx < max_num_players; player_idx++) {

    const auto handle = steam_ui_c.handles[player_idx];
    const bool connected = handle_is_connected(steam_c, handle);
    const bool joined = handle_is_joined(steam_ui_c, handle);

    auto& player_ui_c = ui_c.player_ui_state[player_idx];
    auto& player_state_c = ui_c.player_choice_state[player_idx];

    // always show one player. player_idx either using keyboard or controller
    if ((connected && joined) || player_idx == 0) {
      auto& active_cell = player_ui_c.state.active;
      auto& h_value = dynamic_cast<OptionsCell*>(active_cell.get())->value;
      const ImVec2 button_size = { 100.0f * ui_scaling, 24.0f * ui_scaling };
      const auto& s = player_ui_c.state.actions;
      const bool do_act = std::find(s.begin(), s.end(), UIAction::SELECT) != s.end();
      const bool r_pressed = std::find(s.begin(), s.end(), UIAction::NAV_MOVE_R) != s.end();
      const bool l_pressed = std::find(s.begin(), s.end(), UIAction::NAV_MOVE_L) != s.end();
      if (l_pressed)
        h_value--;
      if (r_pressed)
        h_value++;

      // const auto width = 300;
      const auto height = 420 * ui_scaling; // or 1/6th of the screen

      // if pivot is 0, the top of the ui would be rendered at the center of the screen
      // if pivot is 1. the bot of the ui would be rendered at the center of the screen
      const auto pivot = 0.35f;

      const auto main_quarter_tl = ImVec2{ first_tl_x + x_pad, center_y - (height * pivot) };
      const auto main_quarter_wh = ImVec2{ player_wh.x - 2.0f * x_pad, height };
      const auto main_quarter_br =
        ImVec2{ main_quarter_tl.x + main_quarter_wh.x - x_pad, main_quarter_tl.y + main_quarter_wh.y };

      auto* draw_list = ImGui::GetWindowDrawList();

      const auto my_player_col = default_player_colours[player_idx];
      const auto im_player_col = convert_my_to_im(my_player_col);

      // draw a background
      draw_list->AddRectFilled(main_quarter_tl, main_quarter_br, im_window_bg_col, 6);
      draw_list->AddRect(main_quarter_tl, main_quarter_br, im_player_col, 6, ImDrawFlags_RoundCornersAll, 2);

      // Draw categories + values
      const auto space_between_buttons = 10;
      auto button_tl = main_quarter_tl;
      button_tl.y += 10; // y padding on the first button
      button_tl.x -= 10; // put it outside the frame?
      for (int i = 0; i < player_ui_c.state.cells.size(); i++) {
        auto& base = player_ui_c.state.cells[i];
        const bool active = base == player_ui_c.state.active;

        ImGui::SetCursorPos(button_tl);

        SelectableButtonDef def{
          .label = base->name,
          .size = button_size,
          .input = do_act,
          .cell = base,
          .active_cell = player_ui_c.state.active,
          .font = font,
        };
        if (selectable_button(r, def))
          base->action();

        const float pos_l = main_quarter_tl.x + button_size.x;
        const float pos_w = main_quarter_br.x - pos_l;
        const auto pos = ImVec2{ pos_l + pos_w * 0.5f, button_tl.y };
        draw_selections(r, ui_c, base, player_idx, i, pos, button_size, active);

        if (i + 1 < player_ui_c.state.cells.size())
          button_tl.y += button_size.y + space_between_buttons;
      }

      //
      // begin info section.
      //

      const auto box_tl = ImVec2{ main_quarter_tl.x, button_tl.y + button_size.y + space_between_buttons };
      const auto box_br = main_quarter_br;
      const auto box_wh = ImVec2{ box_br.x - box_tl.x, box_br.y - box_tl.y };
      draw_stats(r, box_tl, box_wh, player_ui_c);

      //
      // draw confirm timer.
      //

      const auto back_str = get_str_for_da(steam_c, handle, DigitalAction::Game_East);
      const auto confirm_str = get_str_for_da(steam_c, handle, DigitalAction::Game_South);

      const auto ready_text = std::format("Hold {} to ready.", confirm_str);
      const auto back_text = std::format("Hold {} for main menu.", back_str);
      const auto desc_pos = ImVec2(box_tl.x + 0.5f * box_wh.x, box_tl.y + 0.10f * box_wh.y);

      // draw a bar that represents ready percentage
      auto my_player_col_active = my_player_col;
      auto my_player_col_inactive = my_player_col;
      my_player_col_inactive.a = 0.25f * 255;
      const auto im_player_col_active = convert_my_to_im(my_player_col_active);
      const auto im_player_col_inactive = convert_my_to_im(my_player_col_inactive);

      const auto text_size = ImGui::CalcTextSize("A");
      const auto draw_bar = [&](ImVec2 pos, const float percent, const float bar_offset) {
        const float bar_height = text_size.y;
        const float bar_width = box_wh.x - 2.0f * bar_offset;
        const float bar_rounding = 10.0f;
        const ImDrawFlags flags = ImDrawFlags_RoundCornersRight;

        // bar bg
        const auto full_bar_tl = pos;
        const auto full_bar_br = ImVec2(full_bar_tl.x + bar_width, full_bar_tl.y + bar_height);
        draw_list->AddRectFilled(full_bar_tl, full_bar_br, im_player_col_inactive, bar_rounding, flags);

        // bar fg
        const auto bar_tl = pos;
        const auto bar_br = ImVec2(bar_tl.x + percent * bar_width, bar_tl.y + bar_height);
        draw_list->AddRectFilled(bar_tl, bar_br, im_player_col_active, bar_rounding, flags);
      };

      const float percent_0 = player_state_c.confirm_held_time / player_state_c.confirm_held_time_max;
      const float percent_1 = player_state_c.back_held_time / player_state_c.back_held_time_max;
      const float bar_offset = (box_wh.x * 0.1f);

      const auto text_tl_0 = ImVec2{ box_tl.x + bar_offset, box_br.y - 40 - 6 };
      const auto text_tl_1 = ImVec2{ box_tl.x + bar_offset, box_br.y + text_size.y - 40 };

      draw_bar(text_tl_0, percent_0, bar_offset);
      ImGui::SetCursorPos(text_tl_0);
      ImGui::Text("%s", ready_text.c_str());

      draw_bar(text_tl_1, percent_1, bar_offset);
      ImGui::SetCursorPos(text_tl_1);
      ImGui::Text("%s", back_text.c_str());

    } else
      break; // no more players

    // move horizontally
    first_tl_x += player_wh.x;
  }

  ImGui::End();
  ImGui::PopStyleVar(2);
  ImGui::PopFont();
};

void
update_ui_scene_select_system(entt::registry& r, const float dt)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif
  auto& ri_c = SINGLE_RendererInfo::instance;
  GET_FIRST_OR_RETURN(SINGLE_SelectSceneData, r, ui_e, ui_c)
  GET_FIRST_OR_RETURN(SINGLE_InputComponent, r, input_e, input_c)
  GET_FIRST_OR_RETURN(SINGLE_Hulls, r, hulls_e, hulls_c)
  GET_FIRST_OR_RETURN(SINGLE_Weapons, r, weapons_e, weapons_c)
  GET_FIRST_OR_RETURN(SINGLE_SteamControllerGameState, r, steam_state_e, steam_state_c)
  GET_FIRST_OR_RETURN(SINGLE_SteamControllers, r, steam_e, steam_c)
  const auto& scene_c = get_first_component<SINGLE_CurrentScene>(r);

  const int max_num_players = 4;
  const auto num_active_players = glm::max(1, (int)steam_c.n_active);

  if (!ui_c.init) {
    ui_c.player_ui_state.resize(max_num_players);
    ui_c.player_choice_state.resize(max_num_players);

    for (int i = 0; i < max_num_players; i++) {
      auto& ui_state_c = ui_c.player_ui_state[i];

      if (!ui_state_c.init) {

        OptionsCell c;
        c.name = "Hull";
        c.action = []() {};
        ui_state_c.state.cells.push_back(std::make_shared<OptionsCell>(c));

        OptionsCell c1;
        c1.name = "Weapon";
        c1.action = []() {};
        ui_state_c.state.cells.push_back(std::make_shared<OptionsCell>(c1));

        OptionsCell c2;
        c2.name = "Ability";
        c2.action = []() {};
        ui_state_c.state.cells.push_back(std::make_shared<OptionsCell>(c2));

        create_as_vertical_layout(ui_state_c.state.cells);

        ui_state_c.state.active = ui_state_c.state.cells[0];
        ui_state_c.init = true;
      }
    }

    ui_c.init = true;
    return;
  }

  update_input_for_select_ui(r, ui_c, dt);

  if (scene_c.s != Scene::select_ships)
    return;

  update_player_select_ui(r, ri_c, ui_c, num_active_players, max_num_players);

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
