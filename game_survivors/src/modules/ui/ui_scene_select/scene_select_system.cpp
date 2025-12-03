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
#include "engine/sprites/helpers.hpp"
#include "engine/std/string/helpers.hpp"
#include "modules/actors/actor_weapon/weapon_components.hpp"
#include "modules/core/fonts/fonts_helpers.hpp"
#include "modules/core/io/io_helpers.hpp"
#include "modules/core/renderer/components.hpp"
#include "modules/core/renderer/helpers.hpp"
#include "modules/core/ui/ui_common_components.hpp"
#include "modules/core/ui/ui_common_helpers.hpp"
#include "modules/events/event_weapon_level_reached/event_weapon_level_reached_helpers.hpp"
#include "modules/scene/scene_components.hpp"
#include "modules/steam_input/steam_input_components.hpp"
#include "modules/steam_input/steam_input_helpers.hpp"
#include "modules/systems/system_hardpoint_arcs/hulls_components.hpp"
#include "modules/ui/ui_colours/ui_colours_helpers.hpp"
#include "modules/ui/ui_element_cursor/element_cursor_helpers.hpp"
#include "modules/ui/ui_popup_options/ui_popup_options_components.hpp"
#include "modules/ui/ui_scene_main_menu_controllerinfo/ui_main_menu_controllerinfo_components.hpp"
#include "modules/ui/ui_scene_main_menu_controllerinfo/ui_main_menu_controllerinfo_helpers.hpp"
#include "modules/ui/ui_scene_survive_onboarding/ui_survive_onboarding_helpers.hpp"
#include "modules/ui/ui_scene_survive_upgrade/ui_survive_upgrade_helpers.hpp"
#include "resources/data.hpp"

namespace game2d {

struct TextDesc
{
  std::string text = "";
  ImVec2 non_centered_pos{ 0, 0 };
  ImU32 col = 0;
  float wrap_width = -1;
  float font_size = 16.0f;
};

void
add_text_centered_here(entt::registry& r, ImDrawList* draw_list, const TextDesc& desc, ImVec2* out_pos = nullptr)
{
  auto* font = get_inter_font(r);
  const auto font_size = desc.font_size;

  const auto text_size = font->CalcTextSizeA(font_size, FLT_MAX, desc.wrap_width, desc.text.c_str());
  const auto text_pos = desc.non_centered_pos - ImVec2{ 0.5f * text_size.x, 0.5f * text_size.y };
  draw_list->AddText(font, font_size, text_pos, desc.col, desc.text.c_str(), NULL, desc.wrap_width);

  if (out_pos) {
    out_pos->x = text_pos.x;
    out_pos->y = text_pos.y;
  }
};

void
update_selections(entt::registry& r,
                  SINGLE_SelectSceneData& ui_c,
                  const std::shared_ptr<Cell>& base,
                  const int player_idx,
                  const ImVec2 pos,
                  const ImVec2 button_size,
                  const bool active,
                  const float dt)
{
  GET_FIRST_OR_RETURN(SINGLE_Hulls, r, hulls_e, hulls_c)
  GET_FIRST_OR_RETURN(SINGLE_Weapons, r, weapons_e, weapons_c)
  auto& player_state = ui_c.player_choice_state[player_idx];

  auto& cell = *(dynamic_cast<OptionsCell*>(base.get()));
  const auto& unlocked_hulls = ui_c.unlocked_hulls;
  const auto& unlocked_weapons = ui_c.unlocked_weapons;

  const bool is_name = cell.name.find("Name") != std::string::npos;
  const bool is_colour = cell.name.find("Colour") != std::string::npos;
  const bool is_hull = cell.name.find("Hull") != std::string::npos;
  const bool is_weapon = cell.name.find("Weapon") != std::string::npos;

  std::string name = "";

  if (is_name) {
    cell.value = engine::wrap(cell.value, (int)ui_c.available_names.size());
    const auto& out_name = ui_c.available_names[cell.value];
    name = out_name;
    player_state.player_name = name;
  }

  if (is_colour) {
    int n_colours = (int)default_player_colours.size();
    cell.value = engine::wrap(cell.value, n_colours);
    name = "";
    player_state.player_colour_idx = cell.value;
  }

  // convert index to hull choice
  if (is_hull) {
    cell.value = engine::wrap(cell.value, (int)unlocked_hulls.size());
    const auto& hull = unlocked_hulls[cell.value];
    name = hull.name;
    player_state.player_boat_key = hull.key;
  }

  // convert index to weapon choice
  if (is_weapon) {
    cell.value = engine::wrap(cell.value, (int)unlocked_weapons.size());
    const auto& weapon = unlocked_weapons[cell.value];
    name = weapon.name.c_str();
    player_state.player_gun_key = weapon.key;
  }

  // convert index to ability choice.
  // if (is_ability)
  //   name = "None";

  // center the text y
  // ImGui::SetCursorPos({ pos.x - 0.5f * text_wh.x, pos.y + 0.5f * (button_size.y - text_wh.y) });

  // if (active) {
  //   ImGui::Text("< ");
  //   ImGui::SameLine();
  //   ImGui::TextColored(im_text_col, "%s", name.c_str());
  //   ImGui::SameLine();
  //   ImGui::Text(" >");
  // } else {
  //   ImGui::TextColored(im_text_disabled_col, "%s", name.c_str());
  // }

  // {
  //   const auto text = active ? std::format("< {} >", name) : name;
  //   const auto text_wh = ImGui::CalcTextSize(text.c_str());
  // }
};

void
draw_stats(entt::registry& r, ImVec2 box_tl, ImVec2 box_wh, SelectUI& player_ui_c)
{
  GET_FIRST_OR_RETURN(SINGLE_Hulls, r, hulls_e, hulls_c)
  GET_FIRST_OR_RETURN(SINGLE_Weapons, r, weapons_e, weapons_c)
  GET_FIRST_OR_RETURN(SINGLE_SelectSceneData, r, scene_e, scene_c)

  const auto box_br = box_tl + box_wh;
  auto* draw_list = ImGui::GetWindowDrawList();

  // add a background
  // draw_list->AddRectFilled(box_tl, box_br, IM_COL32(255, 0, 0, 255), 6);

  const auto& active_cell = player_ui_c.state.active;
  const auto& cs = player_ui_c.state.cells;
  const auto cell_it = std::find(cs.begin(), cs.end(), active_cell);
  const auto cell_idx = static_cast<int>(cell_it - cs.begin());
  auto& cell = *(dynamic_cast<OptionsCell*>(cell_it->get()));
  const bool is_name = cell.name.find("Name") != std::string::npos;
  const bool is_hull = cell.name.find("Hull") != std::string::npos;
  const bool is_weapon = cell.name.find("Weapon") != std::string::npos;

  std::string info_key = "";
  std::string info_desc = "";
  const auto& unlocked_hulls = scene_c.unlocked_hulls;
  const auto& unlocked_weapons = scene_c.unlocked_weapons;

  if (is_hull) {
    cell.value = engine::wrap(cell.value, (int)unlocked_hulls.size());
    const auto& hull = unlocked_hulls[cell.value];
    info_key = hull.name;
    info_desc = hull.desc;
  }
  if (is_weapon) {
    cell.value = engine::wrap(cell.value, (int)unlocked_weapons.size());
    const auto& weapon = unlocked_weapons[cell.value];
    info_key = weapon.name;
    info_desc = weapon.desc;
  }

  const auto head_pos = ImVec2(box_tl.x + 0.5f * box_wh.x, box_tl.y + 0.03f * box_wh.y);
  const auto desc_pos = ImVec2(box_tl.x + 0.5f * box_wh.x, box_tl.y + 0.07f * box_wh.y);
  const auto stat_pos = ImVec2(box_tl.x + 0.5f * box_wh.x, box_tl.y + 0.15f * box_wh.y);
  const auto line_height = 13.0f;

  // header text.
  {
    const auto header = info_key;

    const TextDesc header_text_desc{
      .text = header,
      .non_centered_pos = head_pos,
      .col = IM_COL32(255, 255, 255, 255),
      .wrap_width = -1,
      .font_size = (float)FontSizes::SIZE_16,
    };
    ImVec2 header_text_pos_centered{ 0, 0 };
    add_text_centered_here(r, draw_list, header_text_desc, &header_text_pos_centered);
  }

  // description text.
  {
    const std::string desc = info_desc;

    const auto font_size = (float)FontSizes::SIZE_13;
    auto* font = get_inter_font(r);

    const float width_limit = box_wh.x - (2.0f * 4);
    const auto text_size_x = font->CalcTextSizeA(font_size, width_limit, width_limit, desc.c_str()).x;

    const auto desc_text_pos_centered = ImVec2(desc_pos.x - 0.5f * text_size_x, desc_pos.y);
    draw_list->AddText(font, font_size, desc_text_pos_centered, im_text_col, desc.c_str(), NULL, width_limit);
  }

  // stats.
  {
    std::vector<DisplayStat> display_stats;

    if (is_hull) {
      const auto& hull = hulls_c.hulls[cell.value];
      display_stats.push_back({ .key = "Hardpoints", .val = std::to_string(hull.hardpoints.size()) });
      display_stats.push_back({ .key = "Width", .val = std::format("{:.1f}m", pixels_to_meters(hull.width * 10)) });
      display_stats.push_back({ .key = "Height", .val = std::format("{:.1f}m", pixels_to_meters(hull.height * 10)) });

      const int hp = (int)((hull.width * hull.height) / 100.0f);
      display_stats.push_back({ .key = "HP", .val = std::format("{}", hp) });
    }

    if (is_weapon) {
      const auto& weapon = weapons_c.weapons[cell.value];
      for (const auto& [key, val] : weapon.data) {
        auto stat_enum = magic_enum::enum_cast<UpgradeableStat>(key).value();
        auto clean_key = make_stat_name_pretty_name(key);

        bool is_int = std::floor(val) == val;
        if (is_int)
          display_stats.push_back({ .key = clean_key, .val = std::format("{:.0f}", val), .stat = stat_enum });
        else
          display_stats.push_back({ .key = clean_key, .val = std::format("{:.1f}", val), .stat = stat_enum });
      }

      // hack: if you're a sea turret, you deploy other weapons.
      // show the other weapon stats.
      /*
      if (weapon.key == "weapon_sea_turret") {
        auto deployed_weapon = weapons_c.weapons[0];
        display_stats.push_back({ .key = "", .val = "" });
        display_stats.push_back({ .key = "[DEPLOYS]", .val = deployed_weapon.name });
        for (const auto& [key, val] : deployed_weapon.data) {
          auto clean_key = key;
          clean_key = str_remove_all_occurances(clean_key, "WEAPON_");
          clean_key = str_remove_all_occurances(clean_key, "BULLET_");
          display_stats.push_back({ .key = clean_key, .val = std::format("{:.1f}", val) });
        }
      }
      */
    }

    // calculate the max width of all keys
    const float max_width = calculate_width(display_stats);
    const auto text_wh = ImGui::CalcTextSize("A");
    const float offset = 2.0f;

    for (int idx = 0; idx < (int)display_stats.size(); idx++) {
      const auto& stat = display_stats[idx];

      auto l_stat_pos = stat_pos;
      auto r_stat_pos = stat_pos;
      l_stat_pos.x = box_tl.x + 0.1f * box_wh.x;
      r_stat_pos.x = l_stat_pos.x + max_width + 10;
      l_stat_pos.y += line_height * idx;
      r_stat_pos.y += line_height * idx;

      if (stat.stat.has_value()) {
        const auto stat_enum = stat.stat.value();
        const auto stat_key = std::string(magic_enum::enum_name<UpgradeableStat>(stat_enum));
        const auto icon_key = "ICON_" + stat_key + "_CENTERED";
        auto& ri_c = SINGLE_RendererInfo::instance;
        const auto custom_tex_id = search_for_texture_id_by_texture_path(ri_c, "custom")->id;
        const auto custom_im_id = (ImTextureID)(intptr_t)custom_tex_id;
        const auto [uv_tl, uv_br] = convert_sprite_to_uv(r, icon_key);
        ImGui::SetCursorPos({ l_stat_pos.x - 16, l_stat_pos.y - offset });
        ImGui::Image(custom_im_id, { 16, 16 }, uv_tl, uv_br);
      }

      ImGui::SetCursorPos(l_stat_pos);
      ImGui::Text("%s", stat.key.c_str());

      ImGui::SetCursorPos(r_stat_pos);
      ImGui::Text("%s", stat.val.c_str());
    }
  }
};

void
draw_card_inner(entt::registry& r,
                SINGLE_SelectSceneData& ui_c,
                const SINGLE_SteamControllerGameState& steam_ui_c,
                const SINGLE_SteamMappings& steam_c,
                const InputHandle_t handle,
                SelectUI& player_ui_c,
                HullChoice& player_state_c,
                const int player_idx,
                const int player_col_idx,
                const ImVec2 button_size,
                const bool do_act,
                const ImVec2 main_quarter_tl,
                const ImVec2 main_quarter_br,
                const engine::SRGBColour my_player_col,
                ImDrawList* draw_list,
                ImFont* header_font,
                ImFont* text_font,
                const float header_font_size,
                const float text_font_size,
                const float dt)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif

  const auto& ri_c = SINGLE_RendererInfo::instance;
  const auto custom_tex_id = search_for_texture_id_by_texture_path(ri_c, "custom.png(GL_NEAREST)")->id;
  const auto custom_im_id = (ImTextureID)(void*)(intptr_t)custom_tex_id;
  GET_FIRST_OR_RETURN(SINGLE_Hulls, r, hulls_e, hulls_c)
  GET_FIRST_OR_RETURN(SINGLE_Weapons, r, weapons_e, weapons_c)

  // Draw categories + values
  const auto space_between_buttons = 6;
  auto button_tl = main_quarter_tl;
  button_tl.y += 10; // y padding on the first button
  button_tl.x -= 10; // put it outside the frame?

  for (int i = 0; i < player_ui_c.state.cells.size(); i++) {
    auto& base = player_ui_c.state.cells[i];
    const bool active = base == player_ui_c.state.active;

    ImGui::SetCursorPos(button_tl);
    SelectableButtonDef def{
      .display_str = base->name,
      .imgui_hash = "##" + base->name + std::to_string(player_idx),
      .size = button_size,
      .input = do_act,
      .cell = base,
      .active_cell = player_ui_c.state.active,
      .update_selected_on_mouse_move = true,
      .font = header_font,
      .font_size = header_font_size,
      .rounding = 0.0f,
      .thickness = 1.0f,
    };
    if (selectable_button(r, def))
      base->action();

    auto active_cell = def.active_cell == def.cell;

    const float pos_l = main_quarter_tl.x + button_size.x;
    const float pos_w = main_quarter_br.x - pos_l;
    const auto pos = ImVec2{ pos_l + pos_w * 0.5f, button_tl.y };
    update_selections(r, ui_c, base, player_idx, pos, button_size, active, dt);

    const auto grid_tl = button_tl + ImVec2{ button_size.x, 0 };
    const auto grid_br = ImVec2{ main_quarter_br.x, button_tl.y + button_size.y };
    const auto grid_wh = calc_wh(grid_tl, grid_br);
    // draw_list->AddRect(grid_tl, grid_br, im_white);

    // Draw grid
    {
      auto& cell = *(dynamic_cast<OptionsCell*>(base.get()));
      const bool is_name = cell.name.find("Name") != std::string::npos;
      const bool is_colour = cell.name.find("Colour") != std::string::npos;
      const bool is_hull = cell.name.find("Hull") != std::string::npos;
      const bool is_weapon = cell.name.find("Weapon") != std::string::npos;

      const float icon_sprite = 32;
      const float icon_box_size = 32;
      const float padding_x = 10.0f;
      const auto& unlocked_hulls = ui_c.unlocked_hulls;
      const auto& unlocked_weapons = ui_c.unlocked_weapons;

      if (is_name) {
        const auto& names = ui_c.available_names;

        // auto first_x = grid_tl.x + padding_x;
        // draw_list->AddRect(grid_tl, grid_br, IM_COL32(255, 0, 0, 255));

        const auto col = active ? im_text_col : im_text_col_inactive;
        const auto border_col = active ? im_greenish : im_window_border_col;
        const float arrow_pad = 16;

        const auto name_size = 16;
        const auto name = names[cell.value];
        ImGui::PushFont(text_font, name_size);
        const auto name_len = ImGui::CalcTextSize(name.c_str());
        ImGui::PopFont();
        const auto text_pos = calc_center(grid_tl, grid_wh) - ImVec2{ 0.5f * name_len.x, 0.5f * name_len.y };
        draw_list->AddText(text_font, name_size, text_pos, col, name.c_str());

        // add some < and > arrow.
        draw_list->AddText(text_font, name_size, text_pos - ImVec2{ arrow_pad, 0 }, border_col, "<");
        draw_list->AddText(text_font, name_size, text_pos + ImVec2{ name_len.x + 8, 0 }, border_col, ">");
      }
      if (is_colour) {

        const auto text_size = 16;
        // const auto text = std::string("todo: colour");
        const auto text = std::format("{}/{}", (cell.value + 1), ((int)default_player_colours.size()));
        const auto col = active ? im_text_col : im_text_col_inactive;
        const auto border_col = active ? im_greenish : im_window_border_col;
        const float arrow_pad = 16;

        ImGui::PushFont(text_font, text_size);
        const auto name_len = ImGui::CalcTextSize(text.c_str());
        ImGui::PopFont();
        const auto text_pos = calc_center(grid_tl, grid_wh) - ImVec2{ 0.5f * name_len.x, 0.5f * name_len.y };
        draw_list->AddText(text_font, text_size, text_pos, col, text.c_str());

        // add some < and > arrow.
        draw_list->AddText(text_font, text_size, text_pos - ImVec2{ arrow_pad, 0 }, border_col, "<");
        draw_list->AddText(text_font, text_size, text_pos + ImVec2{ name_len.x + 8, 0 }, border_col, ">");
      }
      if (is_hull) {

        auto first_x = grid_tl.x + padding_x;
        for (int j = 0; j < (int)unlocked_hulls.size(); j++) {
          const auto& hull = unlocked_hulls[j];
          const bool icon_active = j == cell.value;
          const auto border_col = icon_active ? im_greenish : im_window_border_col;

          const auto pos_tl = ImVec2{ first_x, grid_tl.y };
          const auto pos_br = ImVec2{ first_x + icon_box_size, grid_br.y };
          draw_list->AddRectFilled(pos_tl, pos_br, im_window_bg_col);
          draw_list->AddRect(pos_tl, pos_br, border_col);

          if (active_cell && icon_active)
            draw_cursor(r, ui_c.player_cursor_state[player_idx], pos_tl, dt);

          // Draw boat icon
          const auto icon_key = "ICON_"s + to_upper(hull.key) + "_32"s;
          ImGui::SetCursorScreenPos(pos_tl +
                                    ImVec2{ (icon_box_size - icon_sprite) * 0.5f, (icon_box_size - icon_sprite) * 0.5f });
          const auto [image_icon_tl, image_icon_br] = convert_sprite_to_uv(r, icon_key);
          ImGui::Image(custom_im_id, { icon_sprite, icon_sprite }, image_icon_tl, image_icon_br);

          first_x += icon_box_size + padding_x; // move horizontally
        }
      }
      if (is_weapon) {
        const auto& weapons_c = get_first_component<SINGLE_Weapons>(r);

        auto first_x = grid_tl.x + padding_x;
        for (int j = 0; j < (int)unlocked_weapons.size(); j++) {
          const auto& weapon = unlocked_weapons[j];

          if (weapon.useable_by_as_enum != WEAPON_USEABLE_BY::BOATS)
            continue; // not a boat weapon

          const bool icon_active = j == cell.value;
          const auto border_col = icon_active ? im_greenish : im_window_border_col;

          const auto pos_tl = ImVec2{ first_x, grid_tl.y };
          const auto pos_br = ImVec2{ first_x + icon_box_size, grid_br.y };
          draw_list->AddRectFilled(pos_tl, pos_br, im_window_bg_col);
          draw_list->AddRect(pos_tl, pos_br, border_col);

          if (active_cell && icon_active)
            draw_cursor(r, ui_c.player_cursor_state[player_idx], pos_tl, dt);

          // Draw weapon icon
          const auto icon_key = "ICON_"s + to_upper(weapon.key) + "_32";
          ImGui::SetCursorScreenPos(pos_tl +
                                    ImVec2{ (icon_box_size - icon_sprite) * 0.5f, (icon_box_size - icon_sprite) * 0.5f });
          const auto [image_icon_tl, image_icon_br] = convert_sprite_to_uv(r, icon_key);
          ImGui::Image(custom_im_id, { icon_sprite, icon_sprite }, image_icon_tl, image_icon_br);

          first_x += icon_box_size + padding_x; // move horizontally
        }
      }
    }

    button_tl.y += button_size.y + space_between_buttons;

    ImGui::PushFont(text_font, text_font_size);
    const auto text_size_y = ImGui::CalcTextSize("A").y;

    // begin info section.
    // const auto box_tl = ImVec2{ main_quarter_tl.x, button_tl.y + button_size.y + space_between_buttons };
    const int rows = player_ui_c.state.cells.size() + 1;
    const auto offset_y = main_quarter_tl.y + rows * button_size.y;
    const auto box_tl = ImVec2{ main_quarter_tl.x, offset_y };
    const auto box_br = main_quarter_br;
    const auto box_wh = ImVec2{ box_br.x - box_tl.x, box_br.y - box_tl.y };
    // draw_list->AddRect(box_tl, box_br, IM_COL32(255, 0, 0, 255));
    draw_stats(r, box_tl, box_wh, player_ui_c);

    // Display the overclocks
    {
      const auto& active_cell = player_ui_c.state.active;
      const auto& cs = player_ui_c.state.cells;
      const auto cell_it = std::find(cs.begin(), cs.end(), active_cell);
      const auto cell_idx = static_cast<int>(cell_it - cs.begin());
      auto& cell = *(dynamic_cast<OptionsCell*>(cell_it->get()));
      const bool is_hull = cell.name.find("Hull") != std::string::npos;
      const bool is_weapon = cell.name.find("Weapon") != std::string::npos;

      if (is_weapon) {
        const auto& weapons_c = get_first_component<SINGLE_Weapons>(r);
        const auto& weapon = weapons_c.weapons[cell.value].key;
        auto upgrades = get_upgrades_from_weapon_key(r, weapon);
        const auto base_pos = ImGui::GetCursorPos();
        const auto overclock_x = box_tl.x + 0.1f * box_wh.x;
        ImGui::SetCursorPos({ overclock_x, base_pos.y });
        ImGui::Text("Overclocks (Lv 4, 8, 12)");
        for (int i = 0; i < upgrades.size(); i++) {
          ImGui::SetCursorPos({ overclock_x, base_pos.y + (i + 1) * text_size_y });
          ImGui::TextColored(im_inactive_col_vec, "- %s", upgrades[i].display.c_str());
        }
      }
    }

    // draw confirm timer.
    {
      const auto ready_text = std::format("Hold {}", get_confirm_button_str(r, handle));
      const auto back_text = std::format("Hold {}", get_back_button_str(r, handle));
      // const auto desc_pos = ImVec2(box_tl.x + 0.5f * box_wh.x, box_tl.y + 0.10f * box_wh.y);

      // draw a bar that represents ready percentage
      auto my_player_col_active = my_player_col;
      auto my_player_col_inactive = my_player_col;
      my_player_col_inactive.a = 0.25f * 255;
      const auto im_player_col_active = convert_my_to_im(my_player_col_active);
      const auto im_player_col_inactive = convert_my_to_im(my_player_col_inactive);

      const float bar_rounding = 0.0f;
      const auto my_player_col = default_player_colours[player_col_idx];
      const auto im_player_col = convert_my_to_im(my_player_col);

      const auto draw_bar = [&](const ImVec2 bar_tl, const ImVec2 bar_br, const float percent) {
        const ImVec2 bar_wh = bar_br - bar_tl;

        // draw a box around the bar
        draw_list->AddRect(bar_tl, bar_br, im_player_col, bar_rounding, ImDrawFlags_RoundCornersAll, 1);

        // bar bg
        draw_list->AddRectFilled(bar_tl, bar_br, im_player_col_inactive, bar_rounding, ImDrawFlags_RoundCornersAll);

        // bar fg
        float x = bar_tl.x + percent * bar_wh.x;
        const auto partial_bar_br = ImVec2(x, bar_br.y);
        ImU32 col_l = im_player_col_active;
        ImU32 col_r = im_player_col_inactive;
        draw_list->AddRectFilledMultiColor(bar_tl, partial_bar_br, col_r, col_l, col_l, col_r);
      };

      const float percent_0 = player_state_c.confirm_held_time / player_state_c.confirm_held_time_max;
      const float percent_1 = player_state_c.back_held_time / player_state_c.back_held_time_max;
      const float bar_offset = (box_wh.x * 0.1f);

      const auto bar_wh = ImVec2{ box_wh.x, text_size_y };
      const auto bar_tl_0 = ImVec2{ box_tl.x + bar_offset, box_br.y - 40 - 6 };
      const auto bar_tl_1 = ImVec2{ box_tl.x + bar_offset, box_br.y + bar_wh.y - 40 };
      const auto bar_br_0 = ImVec2{ bar_tl_0.x + bar_wh.x - 2.0f * bar_offset, bar_tl_0.y + bar_wh.y };
      const auto bar_br_1 = ImVec2{ bar_tl_1.x + bar_wh.x - 2.0f * bar_offset, bar_tl_1.y + bar_wh.y };

      draw_bar(bar_tl_0, bar_br_0, percent_0);

      const auto tex_id = search_for_texture_id_by_texture_path(ri_c, "kenneynl_gameicons")->id;
      const auto im_id = (ImTextureID)(void*)(intptr_t)tex_id;
      const ImVec2 icon_size{ bar_wh.y, bar_wh.y };

      // tick icon
      {
        ImGui::SetCursorPos(bar_tl_0);
        const auto [icon_tl, icon_br] = convert_sprite_to_uv(r, "ICON_TICK"s);
        ImGui::Image(im_id, icon_size, icon_tl, icon_br);
      }
      const auto ready_text_size = ImGui::CalcTextSize(ready_text.c_str());
      auto center_text_x = 0.5f * (bar_tl_0.x + bar_br_0.x);
      center_text_x -= 0.5f * ready_text_size.x;
      ImGui::SetCursorPos({ center_text_x, bar_tl_0.y });
      ImGui::Text("%s", ready_text.c_str());

      draw_bar(bar_tl_1, bar_br_1, percent_1);
      // cross icon
      {
        ImGui::SetCursorPos(bar_tl_1);
        const auto [icon_tl, icon_br] = convert_sprite_to_uv(r, "ICON_CROSS"s);
        ImGui::Image(im_id, icon_size, icon_tl, icon_br);
      }
      const auto back_text_size = ImGui::CalcTextSize(back_text.c_str());
      auto back_text_x = 0.5f * (bar_tl_1.x + bar_br_1.x);
      back_text_x -= 0.5f * back_text_size.x;
      ImGui::SetCursorPos({ back_text_x, bar_tl_1.y });
      ImGui::Text("%s", back_text.c_str());
      ImGui::PopFont();
    }
  }
}

void
update_player_select_ui(entt::registry& r,
                        SINGLE_RendererInfo& ri_c,
                        SINGLE_SelectSceneData& ui_c,
                        const int num_active_players,
                        const int max_num_players,
                        const float dt)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif

  GET_FIRST_OR_RETURN(SINGLE_SteamControllerGameState, r, steam_ui_e, steam_ui_c);
  GET_FIRST_OR_RETURN(SINGLE_SteamMappings, r, steam_e, steam_c);
  const auto& steam_con_c = get_first_component<SINGLE_SteamConnectedControllers>(r);

  auto* font = get_inter_font(r);
  auto* header_font = font;
  auto* text_font = font;
  const auto header_font_size = (float)FontSizes::SIZE_16;
  const auto text_font_size = (float)FontSizes::SIZE_13;

  const auto set_window_pos = ImVec2{ ri_c.viewport_size_render_at.x * 0.5f, ri_c.viewport_size_render_at.y * 0.5f };
  const auto set_window_size = ImVec2{ (float)ri_c.viewport_size_render_at.x, (float)ri_c.viewport_size_render_at.y };
  ImGui::SetNextWindowPos(set_window_pos, ImGuiCond_Always, { 0.5f, 0.5f });
  ImGui::SetNextWindowSize(set_window_size);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

  imgui_begin("SelectShipUI", ImGuiWindowFlags_NoInputs);
  const ImVec2 window_tl = ImGui::GetWindowPos();
  const ImVec2 window_wh = ImGui::GetWindowSize();
  const ImVec2 player_wh = { window_wh.x / (float)max_num_players, window_wh.y };

  // center it.
  const auto card_width = 300.0f;
  const auto center_x = window_tl.x + 0.5f * window_wh.x;
  const auto center_y = window_tl.y + 0.5f * player_wh.y;

  auto first_tl_x = center_x;
  first_tl_x -= max_num_players * (0.5f * card_width);

  for (int player_idx = 0; player_idx < max_num_players; player_idx++) {

    const auto handle = steam_ui_c.handles_that_want_to_play[player_idx];
    const bool connected = handle_is_connected(steam_con_c, handle);
    const bool joined = handle_is_joined(steam_ui_c, handle);

    // always show one player. player_idx either using keyboard or controller
    // if (player_idx != 0 && !(connected && joined))
    //   break;

    auto& player_ui_c = ui_c.player_ui_state[player_idx];
    auto& player_state_c = ui_c.player_choice_state[player_idx];

    auto& active_cell = player_ui_c.state.active;
    auto& h_value = dynamic_cast<OptionsCell*>(active_cell.get())->value;
    const ImVec2 button_size = { 80.0f, 32.0f };

    const auto& s = player_ui_c.state.actions;
    const bool do_act = std::find(s.begin(), s.end(), UIAction::SELECT) != s.end();
    const bool r_pressed = std::find(s.begin(), s.end(), UIAction::NAV_MOVE_R) != s.end();
    const bool l_pressed = std::find(s.begin(), s.end(), UIAction::NAV_MOVE_L) != s.end();
    if (l_pressed)
      h_value--;
    if (r_pressed)
      h_value++;

    // get player_idx_col value
    int player_col_idx = 0;
    for (const auto& c : player_ui_c.state.cells) {
      const bool is_colour = c->name.find("Colour") != std::string::npos;
      if (!is_colour)
        continue;
      auto& cell = *(dynamic_cast<OptionsCell*>(c.get()));
      cell.value = engine::wrap(cell.value, (int)default_player_colours.size());
      player_col_idx = cell.value;
    }

    const auto my_player_col = default_player_colours[player_col_idx];
    const auto im_player_col = convert_my_to_im(my_player_col);

    auto* draw_list = ImGui::GetWindowDrawList();

    // const auto width = 300;
    const auto height = 480; // or 1/6th of the screen

    // if pivot is 0, the top of the ui would be rendered at the center of the screen
    // if pivot is 1. the bot of the ui would be rendered at the center of the screen
    const auto pivot = 0.35f;
    const auto card_pad_x = 5.0f;
    const auto card_center_x = first_tl_x + (0.5f * card_width);
    const auto card_tl = ImVec2{ card_center_x - 0.5f * card_width + card_pad_x, center_y - (height * pivot) };
    const auto card_br = ImVec2{ card_center_x + 0.5f * card_width - card_pad_x, card_tl.y + height };
    // draw_list->AddRect(card_tl, card_br, im_player_col, 2.0f, ImDrawFlags_RoundCornersAll, 1.0f);

    // draw a background
    draw_list->AddRectFilled(card_tl, card_br, im_window_bg_col, 6);
    draw_list->AddRect(card_tl, card_br, im_player_col, 2.0f, ImDrawFlags_RoundCornersAll, 1.0f);

    if ((joined) || player_idx == 0) {
      draw_card_inner(r,
                      ui_c,
                      steam_ui_c,
                      steam_c,
                      handle,
                      player_ui_c,
                      player_state_c,
                      player_idx,
                      player_col_idx,
                      button_size,
                      do_act,
                      card_tl,
                      card_br,
                      my_player_col,
                      draw_list,
                      header_font,
                      text_font,
                      header_font_size,
                      text_font_size,
                      dt);
    } else {
      const auto text = std::string("N/A\n(Connect in Menu)");
      const auto width = card_br.x - card_tl.x;
      const auto text_size = header_font->CalcTextSizeA(header_font_size, FLT_MAX, -1, text.c_str());

      auto center = ImVec2{
        card_tl.x + 0.5f * (card_br.x - card_tl.x),
        card_tl.y + 0.5f * (card_br.y - card_tl.y),
      };
      center.x -= 0.5f * text_size.x;
      center.y -= 0.5f * text_size.y;
      draw_list->AddText(header_font, header_font_size, center, im_player_col, text.c_str());
    };

    // move horizontally
    first_tl_x += card_width;
  }

  ImGui::End();
  ImGui::PopStyleVar(2);
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
  GET_FIRST_OR_RETURN(SINGLE_SteamMappings, r, steam_e, steam_c)
  const auto& scene_c = SINGLE_CurrentScene::instance;

  const int max_num_players = 4;
  const auto joined_handles = non_zero_handles(steam_state_c.handles_that_want_to_play);
  const auto num_active_players = glm::max(1, (int)joined_handles.size()); // 1 because keyboard

  ui_c.player_cursor_state.resize(num_active_players);
  if (!ui_c.init) {
    ui_c.player_ui_state.resize(max_num_players);
    ui_c.player_choice_state.resize(max_num_players);

    // default players to choose different colours
    for (int i = 0; i < max_num_players; i++)
      ui_c.player_choice_state[0].player_colour_idx = i;

    // get all the available names.
#if defined(USE_STEAM)

    // add your name.
    const auto name = std::string(SteamFriends()->GetPersonaName());
    ui_c.available_names.push_back(name);

    // add your remote player names.
    int session_count = SteamRemotePlay()->GetSessionCount();
    for (int i = 0; i < session_count; i++) {
      auto id = SteamRemotePlay()->GetSessionID(i);
      auto name = SteamRemotePlay()->GetSessionClientName(id);
      ui_c.available_names.push_back(name);
    }

#endif

    std::vector<ShipHullData> unlocked_hulls;
    for (const auto& hull : hulls_c.hulls) {
      if (savefile_get_key(r, hull.key))
        unlocked_hulls.push_back(hull);
    }
    std::vector<Weapon_OnDiskData> unlocked_weapons;
    for (const auto& weapon : weapons_c.weapons) {
      if (savefile_get_key(r, weapon.key) && weapon.useable_by_as_enum == WEAPON_USEABLE_BY::BOATS)
        unlocked_weapons.push_back(weapon);
    }
    ui_c.unlocked_hulls = unlocked_hulls;
    ui_c.unlocked_weapons = unlocked_weapons;

    ui_c.available_names.push_back("Destroyer of Worlds");
    ui_c.available_names.push_back("Kleptomaniac");
    ui_c.available_names.push_back("Passionate Lover");
    ui_c.available_names.push_back("Hat Wearer");
    ui_c.available_names.push_back("Big Chonk");

    for (int i = 0; i < max_num_players; i++) {
      auto& ui_state_c = ui_c.player_ui_state[i];

      if (!ui_state_c.init) {

        OptionsCell c0;
        c0.name = "Name";
        c0.action = []() {};
        ui_state_c.state.cells.push_back(std::make_shared<OptionsCell>(c0));

        {
          OptionsCell c;
          c.name = "Colour";
          c.action = []() {};
          ui_state_c.state.cells.push_back(std::make_shared<OptionsCell>(c));
          ((OptionsCell*)ui_state_c.state.cells[1].get())->value = i; // set default
        }

        OptionsCell c;
        c.name = "Hull";
        c.action = []() {};
        ui_state_c.state.cells.push_back(std::make_shared<OptionsCell>(c));

        OptionsCell c1;
        c1.name = "Weapon";
        c1.action = []() {};
        ui_state_c.state.cells.push_back(std::make_shared<OptionsCell>(c1));

        // OptionsCell c2;
        // c2.name = "Ability";
        // c2.action = []() {};
        // ui_state_c.state.cells.push_back(std::make_shared<OptionsCell>(c2));

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

  update_player_select_ui(r, ri_c, ui_c, num_active_players, max_num_players, dt);

  //
  // Show a countdown timer when all joined players are ready
  //

  const std::vector<HullChoice> c = { ui_c.player_choice_state.begin(),
                                      ui_c.player_choice_state.begin() + num_active_players };

  const auto confirmed = [](const HullChoice& choice) { return choice.confirmed; };
  auto everyone_confirmed = std::all_of(c.begin(), c.end(), confirmed);
  auto someone_confirmed = std::any_of(c.begin(), c.end(), confirmed);
  auto noone_confirmed = std::none_of(c.begin(), c.end(), confirmed);
  everyone_confirmed &= !c.empty();
  someone_confirmed &= !c.empty();
  noone_confirmed &= c.empty();

  ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2{ 0.5f, 0.5f });
  update_countdown(ui_c, everyone_confirmed, dt);
  if (everyone_confirmed) {
    update_countdown_ui(r, ui_c);
    update_countdown_to_next_scene(r, ui_c, hulls_c.hulls);
  }

  ImGui::PopStyleVar();
}

} // namespace game2d
