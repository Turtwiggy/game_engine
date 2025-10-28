#include "pch.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/imgui/ui_imgui_defaults.hpp"
#include "engine/maths/grid.hpp"
#include "engine/sprites/helpers.hpp"
#include "entt/entity/fwd.hpp"
#include "modules/core/fonts/fonts_helpers.hpp"
#include "modules/core/io/io_helpers.hpp"
#include "modules/core/renderer/components.hpp"
#include "modules/core/renderer/helpers.hpp"
#include "modules/core/ui/ui_common_components.hpp"
#include "modules/core/ui/ui_common_helpers.hpp"
#include "modules/systems/system_item_gold/gold_components.hpp"
#include "modules/systems/system_persistent_upgrades/persistent_upgrade_components.hpp"
#include "modules/systems/system_upgrade/upgrade_components.hpp"
#include "modules/ui/ui_colours/ui_colours_helpers.hpp"
#include "modules/ui/ui_element_cursor/element_cursor_components.hpp"
#include "modules/ui/ui_element_cursor/element_cursor_helpers.hpp"
#include "modules/ui/ui_scene_survive_upgrade/ui_survive_upgrade_helpers.hpp"
#include "resources/data.hpp"
#include "ui_scene_upgrades_components.hpp"
#include "ui_scene_upgrades_helpers.hpp"
#include "ui_scene_upgrades_system.hpp"

namespace game2d {
using namespace std::literals;

void
draw_moneybag(entt::registry& r, ImVec2 ui_wh, ImVec2 icon_size, ImFont* header_font)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif

  GET_FIRST_OR_RETURN(SINGLE_PersistentUpgradesMenuUI, r, ui_e, ui_c)
  GET_FIRST_OR_RETURN(SINGLE_PersistentUpgrades, r, upgrade_e, upgrade_c);
  auto& ri_c = SINGLE_RendererInfo::instance;
  auto& gold_c = get_first_component<SINGLE_GoldComponent>(r);
  const auto font_scale = get_first_component<SINGLE_UIScaling>(r).scaling;
  const auto monochrome_tex_id = search_for_texture_id_by_texture_path(ri_c, "monochrome")->id;
  const auto monochrome_im_id = (ImTextureID)(void*)(intptr_t)monochrome_tex_id;

  // draw a moneybag for your gold
  const auto center_x = 0.5f * (ui_wh.x);
  const ImVec2 moneybag_icon_size = { 32 * font_scale, 32 * font_scale };
  const auto moneybag_icon_x = center_x - 0.5 * moneybag_icon_size.x;
  const auto [gold_tl, gold_br] = convert_sprite_to_uv(r, "COINPILE_1"s);
  ImGui::SetCursorPosX(moneybag_icon_x);
  ImGui::Image(monochrome_tex_id, moneybag_icon_size, gold_tl, gold_br, im_gold_col, {});

  // Draw gold amount
  ImGui::PushFont(header_font);
  const auto gold_txt = std::format("{}", gold_c.amount);
  const auto gold_txt_wh = ImGui::CalcTextSize(gold_txt.c_str());
  ImGui::SetCursorPosX(moneybag_icon_x + moneybag_icon_size.x);        // right of icon
  ImGui::SetCursorPosY(0.5f * (moneybag_icon_size.y - gold_txt_wh.y)); // center y
  ImGui::TextColored(im_gold_col, "%s", gold_txt.c_str());
  ImGui::PopFont();
}

void
update_ui_scene_upgrades_system(entt::registry& r, const float dt)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif
  GET_FIRST_OR_RETURN(SINGLE_PersistentUpgradesMenuUI, r, ui_e, ui_c)
  GET_FIRST_OR_RETURN(SINGLE_PersistentUpgrades, r, upgrade_e, upgrade_c);
  auto& ri_c = SINGLE_RendererInfo::instance;
  auto& gold_c = get_first_component<SINGLE_GoldComponent>(r);
  const auto font_scale = get_first_component<SINGLE_UIScaling>(r).scaling;

  if (!ui_c.init)
    ui_c.do_init(r);

  ui_c.update<RequestToShowUpgradesMenu>(r);
  if (!ui_c.open)
    return;

  // process actions.
  process_input_for_ui_all_handles(r, ui_c.state);
  process_input_for_grid(r, ui_c);

  const auto g_input_e = get_first<InputComponent, Persistent>(r);
  const auto& g_input_c = r.get<InputComponent>(g_input_e);
  const auto& b_s = g_input_c.button_s;
  const auto& b_e = g_input_c.button_e;
  const bool hel_sel = std::find(b_s.begin(), b_s.end(), ActionStateEnum::HELD) != b_s.end();
  const bool rel_sel = std::find(b_s.begin(), b_s.end(), ActionStateEnum::RELEASE) != b_s.end();
  const bool do_back = std::find(b_e.begin(), b_e.end(), ActionStateEnum::DOWN) != b_e.end();

  // update the selected stat
  const auto stat_key = ui_c.state.cells[ui_c.grid_idx]->name;
  const auto stat_enum = magic_enum::enum_cast<UpgradeableStat>(stat_key);
  ui_c.selected_stat = stat_enum;

  // back pressed
  if (do_back) {
    back_to_main_menu(r, ui_c);
    return;
  }

  // hold to purchase upgrade
  if (hel_sel)
    ui_c.purchase_time += dt;
  if (!hel_sel)
    ui_c.purchase_time -= dt;
  ui_c.purchase_time = glm::clamp(ui_c.purchase_time, 0.0f, ui_c.purchase_time_max);
  if (hel_sel && ui_c.selected_stat.has_value() && ui_c.purchase_time >= ui_c.purchase_time_max) {
    purchase_upgrade(r, ui_c.selected_stat.value());
    ui_c.purchase_time = 0.0f;
  }

  const auto viewport_tl = ImVec2((float)ri_c.viewport_pos.x, (float)ri_c.viewport_pos.y);
  const auto viewport_wh = ImVec2((float)ri_c.viewport_size_render_at.x, (float)ri_c.viewport_size_render_at.y);
  const auto viewport_wh_half = ImVec2(viewport_wh.x * 0.5f, viewport_wh.y * 0.5f);
  const float size_x = 0.5f * 1280.0f * font_scale;
  const float size_y = 0.4f * 720.0f * font_scale;

  // if pivot is 0, window is at the top at the center of the screen
  // if pivot is 1, window is at the bot at the center of the screen

  static float pivot = 0.5f;
  const float pos_y = viewport_tl.y + 0.5f * viewport_wh.y - size_y * pivot;

  const auto pos = ImVec2(viewport_tl.x + viewport_wh_half.x, pos_y);
  ImGui::SetNextWindowSize(ImVec2(size_x, size_y), ImGuiCond_Always);
  ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(0.5f, 0.0f));
  // ImGui::SetNextWindowSizeConstraints({ 400, 200 }, { 1000, 1000 });

  imgui_begin("upgrades menu");
  auto* draw_list = ImGui::GetWindowDrawList();
  const ImVec2 ui_tl = ImGui::GetWindowPos();
  const ImVec2 ui_wh = ImGui::GetWindowSize();
  const ImVec2 ui_br = { ui_tl.x + ui_wh.x, ui_tl.y + ui_wh.y };

  // fonts
  auto* fingerpaint_font = ImGui::GetIO().Fonts->Fonts[font_scale == 1.0f ? 4 : 5];
  const auto header_font_enum = font_scale == 1.0f ? FontSize::TEXT_SIZE_20 : FontSize::TEXT_SIZE_20_SCALED;
  const auto header_font_size = (float)header_font_enum;
  auto* header_font = get_inter_font(r, header_font_enum);
  const auto text_font_enum = font_scale == 1.0f ? FontSize::TEXT_SIZE_16 : FontSize::TEXT_SIZE_16_SCALED;
  const auto text_font_size = (float)text_font_enum;
  auto* text_font = get_inter_font(r, text_font_enum);
  const auto TEXT_SIZE = text_font->CalcTextSizeA(text_font_size, FLT_MAX, -1, "A");

  // background
  const auto rounding = 6.0f;
  const auto thickness = 2.0f;
  draw_list->AddRectFilled(ui_tl, ui_br, im_window_bg_col, rounding);
  draw_list->AddRect(ui_tl, ui_br, im_window_border_col, rounding, ImDrawFlags_RoundCornersAll, thickness);

  const auto custom_tex_id = search_for_texture_id_by_texture_path(ri_c, "custom")->id;
  const auto custom_im_id = (ImTextureID)(void*)(intptr_t)custom_tex_id;

  const auto box_tl = ImGui::GetCursorScreenPos();
  const auto box_wh = ui_br - box_tl;

  const auto icon_size = ImVec2{ 32 * font_scale, 32 * font_scale };
  draw_moneybag(r, ui_wh, icon_size, header_font);

  const auto box0_tl = box_tl;
  const auto box0_br = box_tl + ImVec2{ 0.66f * box_wh.x, 1.0f * box_wh.y };
  const auto box0_wh = box0_br - box0_tl;
  const auto center_y = box0_tl.y + 0.5f * box0_wh.y;

  const auto box1_tl = ImVec2{ box0_br.x, box0_tl.y };
  const auto box1_br = ui_br;
  // draw_list->AddRectFilled(box0_tl, box0_br, IM_COL32(255, 0, 0, 255), rounding);
  // draw_list->AddRectFilled(box1_tl, box1_br, IM_COL32(0, 255, 0, 255), rounding);

  const float rows = 4.0f; // TODo: fix this being hard coded
  const auto box0_subset_tl = ImVec2{ box0_tl.x, center_y - rows * 0.5f * icon_size.y };
  const auto box0_subset_br = ImVec2{ box0_br.x, center_y + rows * 0.5f * icon_size.y };
  // draw_list->AddRectFilled(box0_subset_tl, box0_subset_br, IM_COL32(0, 0, 255, 255), 0);

  // Draw the selected stat info
  const auto box1_subset_tl = ImVec2{ box1_tl.x + 20, box0_tl.y + 20 };
  const auto box1_subset_br = ImVec2{ box1_br.x - 20, box1_br.y - 20 };
  draw_list->AddRectFilled(box1_subset_tl, box1_subset_br, im_window_bg_col, rounding);
  draw_list->AddRect(box1_subset_tl, box1_subset_br, im_window_border_col, rounding, ImDrawFlags_RoundCornersAll, thickness);

  const int grid_y = get_grid_y(ui_c.state.cells.size(), ui_c.grid_x);
  const auto grid_tl = box0_subset_tl;
  const auto grid_br = box0_subset_br;
  const auto grid_wh = ImVec2{ grid_br.x - grid_tl.x, grid_br.y - grid_tl.y };

  static float pad_x = 16.0f;
  static float pad_y = 16.0f;
  // ImGui::Begin("DebugUI");
  // imgui_draw_float("pad_x", pad_x);
  // imgui_draw_float("pad_y", pad_y);
  // ImGui::End();

  // Draw upgrades in a grid.
  const int valid_amount = (int)ui_c.state.cells.size();
  for (int i = 0; i < ui_c.grid_x * grid_y; i++) {

    const bool active = i < valid_amount;
    if (!active)
      continue; // skip entry

    // the stat
    const auto stat_key = ui_c.state.cells[i]->name;
    const auto stat_enum = magic_enum::enum_cast<UpgradeableStat>(stat_key);
    const auto icon_key = "ICON_" + stat_key;

    const auto [gx, gy] = engine::grid::index_to_grid_position(i, ui_c.grid_x);
    const auto x_hmm = (gx / (float)ui_c.grid_x);
    const auto y_hmm = (gy / (float)grid_y);

    // note: this equally splits the size
    // auto x_pct = grid_tl.x + grid_wh.x * x_hmm;
    // auto y_pct = grid_tl.y + grid_wh.y * y_hmm;
    auto icon_tl = ImVec2{ grid_tl.x + gx * (icon_size.x + pad_x), grid_tl.y + gy * (icon_size.y + pad_y) };

    // center the icon.
    auto cell_w = grid_wh.x / (float)ui_c.grid_x;
    auto cell_h = grid_wh.y / (float)grid_y;
    icon_tl.x += 0.5f * (cell_w - icon_size.x);
    // y_pct += 0.5f * (cell_h - icon_size.y);

    // add background
    const auto rect_min = icon_tl;
    const auto rect_max = icon_tl + icon_size;
    draw_list->AddRectFilled(rect_min, rect_max, im_window_bg_col);
    draw_list->AddRect(rect_min, rect_max, im_window_border_col);

    // add icon
    ImGui::SetCursorScreenPos(icon_tl);
    const auto [image_icon_tl, image_icon_br] = convert_sprite_to_uv(r, icon_key);
    ImGui::Image(custom_im_id, icon_size, image_icon_tl, image_icon_br);

    // update selection with mouse as well
    bool is_hovered = ImGui::IsMouseHoveringRect(rect_min, rect_max);
    const ImVec2 mouse_delta = ImGui::GetIO().MouseDelta;
    const bool mouse_move = mouse_delta.x != 0.0f || mouse_delta.y != 0.0f;
    if (is_hovered && mouse_move)
      ui_c.grid_idx = i;

    // ImGui::SetCursorScreenPos(icon_tl);
    // const auto& cell = ui_c.state.cells[i];
    // const auto name = cell->name;
    // const auto [aquired, total] = get_upgrade_level(r, upgrade_c, name);
    // ImGui::TextColored(im_text_col, "%i/%i", aquired, total);
  }

  // Draw selected cursor
  {
    const auto [gx, gy] = engine::grid::index_to_grid_position(ui_c.grid_idx, ui_c.grid_x);
    auto icon_tl = ImVec2{ grid_tl.x + gx * (icon_size.x + pad_x), grid_tl.y + gy * (icon_size.y + pad_y) };

    // center the cursor
    auto cell_w = grid_wh.x / (float)ui_c.grid_x;
    auto cell_h = grid_wh.y / (float)grid_y;
    icon_tl.x += 0.5f * (cell_w - icon_size.x);

    // only one cursor in the upgrades menu.
    auto& cursor_c = get_first_component<UiCursorComponent>(r);
    draw_cursor(r, cursor_c, icon_tl, dt);
  }

  // Draw selected stat info
  {
    const auto upgrade_tl = box1_subset_tl;
    const auto upgrade_br = box1_subset_br;
    const auto upgrade_wh = upgrade_br - upgrade_tl;
    if (ui_c.selected_stat.has_value()) {
      const auto stat_str = std::string(magic_enum::enum_name<UpgradeableStat>(ui_c.selected_stat.value()));
      const auto display_str = make_stat_name_pretty_name(stat_str);
      const auto display_str_size = text_font->CalcTextSizeA(text_font_size, FLT_MAX, -1, display_str.c_str());

      const auto header_text_pos_tl = ImVec2{ upgrade_tl.x, upgrade_tl.y + 4.0f };
      const auto header_text_pos_adj =
        ImVec2{ header_text_pos_tl.x + 0.5f * (upgrade_wh.x - display_str_size.x), header_text_pos_tl.y };
      draw_list->AddText(text_font, text_font->FontSize, header_text_pos_adj, im_text_col, display_str.c_str());
      // ImGui::TextColored(im_text_col, "Upgrade: %s. Available: %i. Purchased: %i.", u.key.c_str(), total, aquired);

      const auto find_by_key = [&stat_str](Upgrade& u) { return u.key == stat_str; };
      const auto it = std::find_if(upgrade_c.upgrades.begin(), upgrade_c.upgrades.end(), find_by_key);
      if (it == upgrade_c.upgrades.end()) {
        const auto err = std::format("Upgrade does not exist: {} in SINGLE_PersistentUpgrades", stat_str);
        SDL_Log("error: %s", err.c_str());
        // throw std::runtime_error(err.c_str());
        // NO persistent upgrade for this stat
        ImGui::End();
        return;
      }
      const Upgrade u = (*it);
      const auto [aquired, total] = get_upgrade_level(r, upgrade_c, stat_str);

      // loaded on-disk values
      int your_level = 0;
      auto str_opt = savefile_get_key(r, stat_str);
      if (str_opt.has_value())
        str_opt->get_to(your_level);

      const auto tex_id = search_for_texture_id_by_texture_path(ri_c, "kenneynl_gameicons")->id;
      const auto im_id = (ImTextureID)(void*)(intptr_t)tex_id;
      const ImVec2 upg_icon_size{ TEXT_SIZE.y, TEXT_SIZE.y };

      // display the current upgrade level, and future upgrade levels
      for (int i = 0; i < u.levels.size(); i++) {
        const UpgradeLevel& l = u.levels[i];
        const bool aquired = i < your_level;

        std::string str = "";
        if (l.type == "stat_percent_increase")
          str = std::format("{}G. +{}%", l.cost, l.value, aquired);
        else if (l.type == "stat_flat_increase")
          str = std::format("{}G. +{}", l.cost, l.value, aquired);

        ImVec2 icon_pos = header_text_pos_tl;
        icon_pos.y += (i + 1) * TEXT_SIZE.y;

        ImVec2 text_pos = icon_pos;
        text_pos.x += upg_icon_size.x;

        if (aquired) {
          // tick icon
          {
            const auto [icon_tl, icon_br] = convert_sprite_to_uv(r, "ICON_TICK"s);
            draw_list->AddImage(im_id, icon_pos, icon_pos + upg_icon_size, icon_tl, icon_br);
          }
        } else {
          // circle icon
          {
            // const auto [icon_tl, icon_br] = convert_sprite_to_uv(r, "ICON_CROSS"s);
            // draw_list->AddImage(im_id, icon_pos, icon_pos + upg_icon_size, icon_tl, icon_br);
            auto center = icon_pos + ImVec2{ 0.5f * upg_icon_size.x, 0.5f * upg_icon_size.y };
            draw_list->AddCircle(center, 0.33f * upg_icon_size.x, im_text_col, 16, 2.0f);
          }
        }

        if (aquired)
          draw_list->AddText(text_font, text_font->FontSize, text_pos, aquired_col, str.c_str());
        else
          draw_list->AddText(text_font, text_font->FontSize, text_pos, unaquired_col, str.c_str());

        //
      }

      // todo: draw a "hold to aquire" button
    }
  }

  // Draw purchase bar.
  {
    const auto purchasebar_tl = ImVec2{ box1_subset_tl.x + 5.0f, box1_subset_br.y - 25.0f };
    const auto purchasebar_br = ImVec2{ box1_subset_br.x - 5.0f, box1_subset_br.y - 5.0f };
    const auto purchasebar_wh = purchasebar_br - purchasebar_tl;

    const engine::SRGBColour my_player_col = default_player_colours[0];
    auto my_player_col_active = my_player_col;
    auto my_player_col_inactive = my_player_col;
    my_player_col_inactive.a = 0.25f * 255;
    const auto im_player_col_active = convert_my_to_im(my_player_col_active);
    const auto im_player_col_inactive = convert_my_to_im(my_player_col_inactive);
    const auto im_player_col = convert_my_to_im(my_player_col);
    const float bar_rounding = 0.0f;

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

    const float percent = ui_c.purchase_time / ui_c.purchase_time_max;
    draw_bar(purchasebar_tl, purchasebar_br, percent);

    const auto text = "Hold to Purchase"s;
    const auto text_size = text_font->CalcTextSizeA(text_font->FontSize, FLT_MAX, -1, text.c_str());
    const auto text_pos = ImVec2{ purchasebar_tl.x + 0.5f * (purchasebar_wh.x - text_size.x),
                                  purchasebar_tl.y + 0.5f * (purchasebar_wh.y - text_size.y) };
    draw_list->AddText(text_font, text_font->FontSize, text_pos, im_text_col, text.c_str());

    //
  }

  ImGui::End();
}

} // namespace game2d