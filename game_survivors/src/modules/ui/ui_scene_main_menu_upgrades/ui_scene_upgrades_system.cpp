#include "pch.hpp"

#include "engine/deps/opengl.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/imgui/helpers.hpp"
#include "engine/imgui/ui_imgui_defaults.hpp"
#include "engine/maths/grid.hpp"
#include "engine/opengl/texture.hpp"
#include "engine/sprites/helpers.hpp"
#include "engine/std/string/helpers.hpp"
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
draw_moneybag(entt::registry& r, ImVec2 ui_tl, ImVec2 ui_wh, ImFont* header_font, float header_font_size)
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
  auto* draw_list = ImGui::GetWindowDrawList();

  const auto icon_size = ImVec2{ 16, 16 };
  const auto inset = ImVec2{ 8, 8 };
  const auto icon_tl = ImVec2{ ui_tl.x + inset.x, ui_tl.y + inset.y };
  const auto icon_br = ImVec2{ ui_tl.x + inset.x + icon_size.x, ui_tl.y + inset.y + icon_size.y };

  // draw a moneybag for your gold
  const auto [uv_tl, uv_br] = convert_sprite_to_uv(r, "COINPILE_1"s);
  draw_list->AddImage(monochrome_im_id, icon_tl, icon_br, uv_tl, uv_br, im_gold_col);

  // draw the amount of gold you have.
  const auto gold_txt = std::format(" {}", gold_c.amount);
  draw_list->AddText(header_font, header_font_size, ImVec2{ icon_br.x, icon_tl.y }, im_text_col, gold_txt.c_str());
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

  if (!ui_c.init)
    ui_c.do_init(r);

  ui_c.update<RequestToShowUpgradesMenu>(r);
  if (!ui_c.open)
    return;

  // process actions.
  process_input_for_ui_all_handles(r, ui_c.state);

  const auto g_input_e = get_first<InputComponent, Persistent>(r);
  const auto& g_input_c = r.get<InputComponent>(g_input_e);
  const auto& b_s = g_input_c.button_s;
  const auto& b_e = g_input_c.button_e;
  bool hel_sel = std::find(b_s.begin(), b_s.end(), ActionStateEnum::HELD) != b_s.end();
  const bool rel_sel = std::find(b_s.begin(), b_s.end(), ActionStateEnum::RELEASE) != b_s.end();
  const bool do_back = std::find(b_e.begin(), b_e.end(), ActionStateEnum::DOWN) != b_e.end();
  const bool do_act = std::find(b_s.begin(), b_s.end(), ActionStateEnum::DOWN) != b_s.end();

  // also held if mouse lmb clicked.
  hel_sel |= ImGui::IsMouseDown(ImGuiMouseButton_Left);

  // update the selected stat
  // const auto stat_key = .value()->name;
  // const auto stat_enum = magic_enum::enum_cast<UpgradeableStat>(stat_key);
  // ui_c.selected_stat = stat_enum;

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
  if (hel_sel && ui_c.state.active != nullptr && ui_c.purchase_time >= ui_c.purchase_time_max) {
    const auto& cell = ui_c.state.active;
    const auto stat_enum = magic_enum::enum_cast<UpgradeableStat>(cell->name).value();
    purchase_upgrade(r, stat_enum);
    ui_c.purchase_time = 0.0f;
  }

  // fonts
  auto* fingerpaint_font = get_fingerpaint_font(r);
  auto* inter_font = get_inter_font(r);
  auto* text_font = inter_font;
  const auto font_header_size = (float)FontSizes::SIZE_20;
  const auto font_text_size = (float)FontSizes::SIZE_16;

  const auto viewport_tl = ImVec2((float)ri_c.viewport_pos.x, (float)ri_c.viewport_pos.y);
  const auto viewport_wh = ImVec2((float)ri_c.viewport_size_render_at.x, (float)ri_c.viewport_size_render_at.y);
  const auto viewport_wh_half = ImVec2(viewport_wh.x * 0.5f, viewport_wh.y * 0.5f);
  const float size_x = 0.25f * 1280.0f;
  const float size_y = 450.0f;

  // if pivot is 0, window is at the top at the center of the screen
  // if pivot is 1, window is at the bot at the center of the screen
  const float pivot = 0.32f;
  const float pos_y = viewport_tl.y + 0.5f * viewport_wh.y - size_y * pivot;
  const auto pos = ImVec2(viewport_tl.x + viewport_wh_half.x, pos_y);
  ImGui::SetNextWindowSize(ImVec2(size_x, size_y), ImGuiCond_Always);
  ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(0.5f, 0.0f));
  // ImGui::SetNextWindowSizeConstraints({ 400, 200 }, { 1000, 1000 });

  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
  imgui_begin("upgrades menu");
  {
    auto* draw_list = ImGui::GetWindowDrawList();
    const ImVec2 ui_tl = ImGui::GetWindowPos();
    const ImVec2 ui_wh = ImGui::GetWindowSize();
    const ImVec2 ui_br = { ui_tl.x + ui_wh.x, ui_tl.y + ui_wh.y };

    // background
    const auto rounding = 6.0f;
    const auto thickness = 2.0f;
    draw_list->AddRectFilled(ui_tl, ui_br, im_window_bg_col, rounding);
    draw_list->AddRect(ui_tl, ui_br, im_window_border_col, rounding, ImDrawFlags_RoundCornersAll, thickness);

    // tagline
    ImGui::PushFont(text_font, font_text_size);
    auto tagline = "Shipyard - Get good. And upgrade!"s;
    auto tagline_size = ImGui::CalcTextSize(tagline.c_str());
    ImGui::SetCursorPosX(ui_wh.x * 0.5f - tagline_size.x * 0.5f);
    ImGui::Text("%s", tagline.c_str());
    ImGui::PopFont();

    // moneybag
    draw_moneybag(r, ui_tl, ui_wh, inter_font, font_text_size);

    // draw available upgrades.
    ImGui::NewLine();
    const auto custom_tex_id = search_for_texture_id_by_texture_path(ri_c, "custom")->id;
    const auto custom_im_id = (ImTextureID)(intptr_t)custom_tex_id;
    float start_y = ui_tl.y + 16 * 2;
    for (int i = 0; i < (int)ui_c.state.cells.size(); i++) {
      auto& base = ui_c.state.cells[i];

      // which is the active cell index
      const auto cell_it = std::find(ui_c.state.cells.begin(), ui_c.state.cells.end(), ui_c.state.active);
      const auto cell_idx = static_cast<int>(cell_it - ui_c.state.cells.begin());
      const int col_idx = 0;
      const int row_idx = cell_idx;
      const bool active = base == ui_c.state.active;

      // the stat
      const auto stat_key = base->name;
      const auto stat_enum = magic_enum::enum_cast<UpgradeableStat>(stat_key);
      const auto icon_key = "ICON_" + stat_key + "_CENTERED";
      const auto display_txt = make_stat_name_pretty_name(stat_key);

      auto a_def = SelectableButtonDef{
        .display_str = display_txt,
        .imgui_hash = "##" + base->name,
        .size = { ui_tl.x, 16 },
        .input = do_act,
        .cell = base,
        .active_cell = ui_c.state.active,

        .text_pivot = { 0.0f, 0.5f }, // center_y
        .text_offset = { 32, 0 },
        .font = inter_font,
        .font_size = 13,

        // hide the buttons
        .active_outline_col = { 0.6f, 0.0f, 0.0f, 1.0f },
        .inactive_outline_col = { 0.0f, 0.0f, 0.0f, 0.0f },
        .active_bg_col = { 0.3f, 0.3f, 0.3f, 0.0f },
        .inactive_bg_col = { 0.0f, 0.0f, 0.0f, 0.0f },
      };

      if (selectable_button(r, a_def))
        base->action();

      // add icon
      auto icon_tl = ImVec2(ui_tl.x + 5, start_y);
      auto icon_br = ImVec2(ui_tl.x + 5 + 16, start_y + 16);
      const auto [uv_tl, uv_br] = convert_sprite_to_uv(r, icon_key);
      draw_list->AddImage(custom_im_id, icon_tl, icon_br, uv_tl, uv_br, im_white);

      // add how many of this type you've aquired.
      const auto name = base->name;
      const auto [aquired, total] = get_upgrade_level(r, upgrade_c, name);
      ImGui::PushFont(text_font, 13);
      const auto text = std::format("{}/{}", aquired, total);
      const auto text_size = ImGui::CalcTextSize(text.c_str());
      const auto text_pos = ImVec2{ ui_tl.x + ui_wh.x - text_size.x - 5, start_y };
      draw_list->AddText(text_pos, im_text_col, text.c_str());
      ImGui::PopFont();

      start_y += 16;
    }
  }
  ImGui::End();
  ImGui::PopStyleVar();

  ImGui::SetNextWindowSize(ImVec2(size_x, size_y * 0.5f), ImGuiCond_Always);
  ImGui::SetNextWindowPos({ pos.x + size_x + 10, pos.y + size_y * 0.5f }, ImGuiCond_Always, ImVec2(0.5f, 0.0f));

  imgui_begin("stats menu");
  {
    auto* draw_list = ImGui::GetWindowDrawList();
    const ImVec2 ui_tl = ImGui::GetWindowPos();
    const ImVec2 ui_wh = ImGui::GetWindowSize();
    const ImVec2 ui_br = { ui_tl.x + ui_wh.x, ui_tl.y + ui_wh.y };

    // background
    const auto rounding = 6.0f;
    const auto thickness = 2.0f;
    draw_list->AddRectFilled(ui_tl, ui_br, im_window_bg_col, rounding);
    draw_list->AddRect(ui_tl, ui_br, im_window_border_col, rounding, ImDrawFlags_RoundCornersAll, thickness);

    const auto cell_it = std::find(ui_c.state.cells.begin(), ui_c.state.cells.end(), ui_c.state.active);
    const auto cell_idx = static_cast<int>(cell_it - ui_c.state.cells.begin());
    const auto& cell = ui_c.state.cells[cell_idx];

    const auto stat_str = std::string(cell->name);
    const auto display_str = make_stat_name_pretty_name(stat_str);
    const auto display_str_size = text_font->CalcTextSizeA(font_text_size, FLT_MAX, -1, display_str.c_str());

    const auto header_tl = ImVec2{ ui_tl.x, ui_tl.y + 4.0f };
    const auto header_adj = ImVec2{ header_tl.x + 0.5f * (ui_wh.x - display_str_size.x), header_tl.y };
    draw_list->AddText(text_font, font_text_size, header_adj, im_text_col, display_str.c_str());

    const auto find_by_key = [&stat_str](Upgrade& u) { return u.key == stat_str; };
    const auto it = std::find_if(upgrade_c.upgrades.begin(), upgrade_c.upgrades.end(), find_by_key);
    if (it == upgrade_c.upgrades.end()) {
      const auto err = std::format("Upgrade does not exist: {} in SINGLE_PersistentUpgrades", stat_str);
      SDL_Log("error: %s", err.c_str());
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
    const ImVec2 upg_icon_size{ 16, 16 };

    // display the current upgrade level, and future upgrade levels
    for (int i = 0; i < u.levels.size(); i++) {
      const UpgradeLevel& l = u.levels[i];
      const bool aquired = i < your_level;

      std::string str = "";
      if (l.type == "stat_percent_increase")
        str = std::format("{}G. +{}%", l.cost, l.value, aquired);
      else if (l.type == "stat_flat_increase")
        str = std::format("{}G. +{}", l.cost, l.value, aquired);

      ImVec2 icon_pos = header_adj;
      icon_pos.y += (i + 1) * 16;

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
        draw_list->AddText(text_font, font_text_size, text_pos, aquired_col, str.c_str());
      else
        draw_list->AddText(text_font, font_text_size, text_pos, unaquired_col, str.c_str());

      //
    }

    // Draw purchase bar.
    {
      const auto purchasebar_tl = ImVec2{ ui_tl.x + 5.0f, ui_br.y - 25.0f };
      const auto purchasebar_br = ImVec2{ ui_br.x - 5.0f, ui_br.y - 5.0f };
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
      const auto text_size = text_font->CalcTextSizeA(font_text_size, FLT_MAX, -1, text.c_str());
      const auto text_pos = ImVec2{ purchasebar_tl.x + 0.5f * (purchasebar_wh.x - text_size.x),
                                    purchasebar_tl.y + 0.5f * (purchasebar_wh.y - text_size.y) };
      draw_list->AddText(text_font, font_text_size, text_pos, im_text_col, text.c_str());

      //
    }
  }

  ImGui::End();
}

} // namespace game2d