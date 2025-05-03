#include "pch.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/imgui/ui_imgui_defaults.hpp"
#include "engine/maths/grid.hpp"
#include "engine/sprites/helpers.hpp"
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
#include "modules/ui/ui_scene_main_menu/ui_scene_main_menu_components.hpp"
#include "resources/data.hpp"
#include "ui_scene_upgrades_components.hpp"
#include "ui_scene_upgrades_system.hpp"

namespace game2d {
using namespace std::literals;

void
back_to_main_menu(entt::registry& r, SINGLE_PersistentUpgradesMenuUI& ui_c)
{
  ui_c.open = false;
  ui_c.one_frame_buffer = true;
  ui_c.selected_stat = std::nullopt;
  ui_c.grid_idx = 0;
  create_empty<RequestToShowMainMenu>(r);
}

void
purchase_upgrade(entt::registry& r, const UpgradeableStat stat)
{
  GET_FIRST_OR_RETURN(SINGLE_PersistentUpgrades, r, upgrade_e, upgrade_c);
  GET_FIRST_OR_RETURN(SINGLE_GoldComponent, r, gold_e, gold_c);

  const auto stat_str = std::string(magic_enum::enum_name<UpgradeableStat>(stat));

  const auto find_by_key = [&stat_str](Upgrade& u) { return u.key == stat_str; };
  const auto it = std::find_if(upgrade_c.upgrades.begin(), upgrade_c.upgrades.end(), find_by_key);
  if (it == upgrade_c.upgrades.end()) {
    auto err = std::format("Upgrade does not exist: {}", stat_str);
    throw std::runtime_error(err.c_str());
  }
  const Upgrade u = (*it);
  SDL_Log("You want to purchase: %s. It has %zu levels available", u.key.c_str(), u.levels.size());

  // your current level.
  std::optional<int> your_level = std::nullopt;

  const auto ondisk_opt = savefile_get_key(r, stat_str);
  if (ondisk_opt.has_value()) {
    const auto ondisk_json = ondisk_opt.value();
    ondisk_json.get_to<int>(your_level.emplace());
    SDL_Log("Your current stat level is: %i", your_level.value());
  }

  std::optional<UpgradeLevel> next_ul = std::nullopt;

  // you haven't aquired any levels.
  if (your_level == std::nullopt) {
    if (u.levels.size() > 0)
      next_ul = u.levels[0]; // the first upgrade
  }

  // you have aquired some levels.
  if (your_level != std::nullopt && your_level.value() < u.levels.size())
    next_ul = u.levels[your_level.value()];

  // check: you've already bought the skill-tree to max
  if (!next_ul.has_value()) {
    SDL_Log("Skill-tree at max");
    return;
  }

  // check: you have enough gold.
  if (next_ul.value().cost > gold_c.amount) {
    SDL_Log("Need more gold.");
    return;
  }

  if (your_level.has_value())
    savefile_put_key(r, stat_str, your_level.value() + 1);
  else
    savefile_put_key(r, stat_str, 1);

  // Buy the skill.
  gold_c.amount -= (int)(next_ul.value().cost);

  savefile_put_key(r, "GOLD_AMOUNT", gold_c.amount);
  savefile_save_disk(r);
};

std::pair<int, int>
get_upgrade_level(entt::registry& r, SINGLE_PersistentUpgrades& upgrade_c, std::string stat_key)
{
  int n_stat_upgrades_aquired = 0;
  const auto on_disk_stat_level_opt = savefile_get_key(r, stat_key);
  if (on_disk_stat_level_opt.has_value()) {
    const nlohmann::json data = on_disk_stat_level_opt.value();
    data.get_to(n_stat_upgrades_aquired);
  }

  int n_stat_upgrades = 0;
  const auto find_by_key = [&stat_key](Upgrade& u) { return u.key == stat_key; };
  const auto it = std::find_if(upgrade_c.upgrades.begin(), upgrade_c.upgrades.end(), find_by_key);
  if (it != upgrade_c.upgrades.end())
    n_stat_upgrades = (int)((*it).levels.size());

  // remove ACTOR_ from the display key
  // auto display_stat_key = row.col_name;
  // const std::string str_to_remove = "ACTOR_";
  // const auto actor_pos = display_stat_key.find(str_to_remove);
  // if (actor_pos != std::string::npos)
  //   display_stat_key = display_stat_key.substr(str_to_remove.length(), display_stat_key.length());

  // Prefix the button with the your_aquired out of available_aquired
  // display_stat_key
  // std::string button_str = std::format("{}/{}", n_stat_upgrades_aquired, n_stat_upgrades);
  return { n_stat_upgrades_aquired, n_stat_upgrades };
};

void
process_input_for_grid(entt::registry& r, SINGLE_PersistentUpgradesMenuUI& ui_c)
{
  auto& acts = ui_c.state.actions;
  const auto val_u = std::find(acts.begin(), acts.end(), UIAction::NAV_MOVE_U) != acts.end();
  const auto val_d = std::find(acts.begin(), acts.end(), UIAction::NAV_MOVE_D) != acts.end();
  const auto val_l = std::find(acts.begin(), acts.end(), UIAction::NAV_MOVE_L) != acts.end();
  const auto val_r = std::find(acts.begin(), acts.end(), UIAction::NAV_MOVE_R) != acts.end();

  const auto [cur_x, cur_y] = engine::grid::index_to_grid_position(ui_c.grid_idx, ui_c.grid_x);
  auto new_x = cur_x;
  auto new_y = cur_y;

  if (val_r)
    new_x++;
  if (val_l)
    new_x--;
  if (val_u)
    new_y++;
  if (val_d)
    new_y--;

  int max_x = ui_c.grid_x - 1;
  int max_y = ((int)ui_c.state.cells.size() / ui_c.grid_x) - 1;

  new_x = glm::clamp(new_x, 0, max_x);
  new_y = glm::clamp(new_y, 0, max_y);

  ui_c.grid_idx = engine::grid::grid_position_to_index({ new_x, new_y }, ui_c.grid_x);
}

void
update_ui_scene_upgrades_system(entt::registry& r)
{
  GET_FIRST_OR_RETURN(SINGLE_PersistentUpgradesMenuUI, r, ui_e, ui_c)
  GET_FIRST_OR_RETURN(SINGLE_RendererInfo, r, ri_e, ri)
  GET_FIRST_OR_RETURN(SINGLE_PersistentUpgrades, r, upgrade_e, upgrade_c);
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
  const bool do_sel = std::find(b_s.begin(), b_s.end(), ActionStateEnum::DOWN) != b_s.end();
  const bool do_back = std::find(b_e.begin(), b_e.end(), ActionStateEnum::DOWN) != b_e.end();

  // update the selected stat
  const auto stat_key = ui_c.state.cells[ui_c.grid_idx]->name;
  const auto stat_enum = magic_enum::enum_cast<UpgradeableStat>(stat_key);
  ui_c.selected_stat = stat_enum;

  // back pressed and no selected stat
  if (do_back) {
    back_to_main_menu(r, ui_c);
    return;
  }

  // select pressed and a stat selected
  if (do_sel && ui_c.selected_stat.has_value())
    purchase_upgrade(r, ui_c.selected_stat.value());

  const auto viewport_tl = ImVec2((float)ri.viewport_pos.x, (float)ri.viewport_pos.y);
  const auto viewport_wh = ImVec2((float)ri.viewport_size_render_at.x, (float)ri.viewport_size_render_at.y);
  const auto viewport_wh_half = ImVec2(viewport_wh.x * 0.5f, viewport_wh.y * 0.5f);
  const float size_x = (1.5f / 3.0f) * 1280.0f * font_scale;
  const float size_y = (1.5f / 3.0f) * 720.0f * font_scale;
  const auto pos = ImVec2(viewport_tl.x + viewport_wh_half.x, viewport_tl.y + (viewport_wh.y - size_y) * 0.5f);
  ImGui::SetNextWindowSize(ImVec2(size_x, size_y), ImGuiCond_Always);
  ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(0.5f, 0.0f));
  // ImGui::SetNextWindowSizeConstraints({ 400, 200 }, { 1000, 1000 });

  imgui_begin("upgrades menu");
  auto* draw_list = ImGui::GetWindowDrawList();
  const ImVec2 ui_tl = ImGui::GetWindowPos();
  const ImVec2 ui_wh = ImGui::GetWindowSize();
  const ImVec2 ui_br = { ui_tl.x + ui_wh.x, ui_tl.y + ui_wh.y };

  // fonts
  auto* fingerpaint_font = ImGui::GetIO().Fonts->Fonts[font_scale == 1.0f ? 3 : 4];
  const auto header_font_enum = font_scale == 1.0f ? FontSize::TEXT_SIZE_20 : FontSize::TEXT_SIZE_20_SCALED;
  const auto header_font_size = (float)header_font_enum;
  auto* header_font = get_inter_font(r, header_font_enum);
  const auto text_font_enum = font_scale == 1.0f ? FontSize::TEXT_SIZE_16 : FontSize::TEXT_SIZE_16_SCALED;
  const auto text_font_size = (float)text_font_enum;
  auto* text_font = get_inter_font(r, text_font_enum);
  const auto TEXT_SIZE = text_font->CalcTextSizeA(text_font_size, FLT_MAX, -1, "A");

  const auto rounding = 12.0f;
  const auto thickness = 2.0f;
  const auto rect_flags = ImDrawFlags_RoundCornersAll;
  draw_list->AddRectFilled(ui_tl, ui_br, im_window_bg_col, rounding);

  // draw a moneybag for your gold
  const auto tex_id = search_for_texture_id_by_texture_path(ri, "monochrome")->id;
  const auto im_id = reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(tex_id));
  const ImVec2 icon_size{ 32 * font_scale, 32 * font_scale };
  const auto [gold_tl, gold_br] = convert_sprite_to_uv(r, "COINPILE_1"s);
  ImGui::Image(im_id, icon_size, gold_tl, gold_br, im_gold_col);

  // header
  const auto upgr_text = "Shipyard - for all your shipping needs.";
  const auto upgr_size = fingerpaint_font->CalcTextSizeA(fingerpaint_font->FontSize, FLT_MAX, -1, upgr_text);
  ImGui::SetCursorPos({ (ui_wh.x - upgr_size.x) * 0.5f, upgr_size.y * 0.5f });
  ImGui::PushFont(fingerpaint_font);
  ImGui::TextColored(im_text_col, "%s", upgr_text);
  ImGui::PopFont();

  ImGui::PushFont(text_font);

  // Draw gold amount under the gold moneybagz.
  ImGui::SetCursorPos({ 0, icon_size.y });
  ImGui::TextColored(im_gold_col, "Gold: %i", gold_c.amount);

  ImGui::PushStyleColor(ImGuiCol_Separator, im_separator_col);
  ImGui::Separator();
  ImGui::PopStyleColor();

  const int grid_y = (int)ui_c.state.cells.size() / ui_c.grid_x;
  const auto grid_tl = ImGui::GetCursorScreenPos();
  const auto grid_br = ImVec2{ ui_br.x, grid_tl.y + icon_size.y * grid_y };
  const auto grid_wh = ImVec2{ grid_br.x - grid_tl.x, grid_br.y - grid_tl.y };

  // Draw selected
  {
    const auto [gx, gy] = engine::grid::index_to_grid_position(ui_c.grid_idx, ui_c.grid_x);
    const auto x_hmm = (gx / (float)ui_c.grid_x);
    const auto y_hmm = (gy / (float)grid_y);
    auto x_pct = grid_tl.x + grid_wh.x * x_hmm;
    auto y_pct = grid_tl.y + grid_wh.y * y_hmm;

    // center the cursor
    auto cell_w = grid_wh.x / (float)ui_c.grid_x;
    auto cell_h = grid_wh.y / (float)grid_y;
    x_pct += 0.5f * (cell_w - icon_size.x);
    // y_pct += 0.5f * (cell_h - icon_size.y);

    ImGui::SetCursorScreenPos(ImVec2{ x_pct, y_pct });

    const auto [cursor_tl, cursor_br] = convert_sprite_to_uv(r, "CURSOR_1"s);
    ImGui::Image(im_id, icon_size, cursor_tl, cursor_br, ImVec4(1.0, 0.0, 0.0, 1.0));
  }

  // Draw upgrades in a grid.
  const int valid_amount = (int)ui_c.state.cells.size();
  for (int i = 0; i < ui_c.grid_x * grid_y; i++) {
    const bool active = i < valid_amount;
    if (!active)
      continue; // skip entry

    const auto [gx, gy] = engine::grid::index_to_grid_position(i, ui_c.grid_x);
    const auto x_hmm = (gx / (float)ui_c.grid_x);
    const auto y_hmm = (gy / (float)grid_y);
    auto x_pct = grid_tl.x + grid_wh.x * x_hmm;
    auto y_pct = grid_tl.y + grid_wh.y * y_hmm;

    // center the icon.
    auto cell_w = grid_wh.x / (float)ui_c.grid_x;
    auto cell_h = grid_wh.y / (float)grid_y;
    x_pct += 0.5f * (cell_w - icon_size.x);
    // y_pct += 0.5f * (cell_h - icon_size.y);

    ImGui::SetCursorScreenPos(ImVec2{ x_pct, y_pct });
    const auto [icon_tl, icon_br] = convert_sprite_to_uv(r, "AMMO_BOX"s);
    ImGui::Image(im_id, icon_size, icon_tl, icon_br, im_icon_col);

    // update selection with mouse as well
    const auto is_hovered = ImGui::IsItemHovered();
    const ImVec2 mouse_delta = ImGui::GetIO().MouseDelta;
    const bool mouse_move = mouse_delta.x != 0.0f || mouse_delta.y != 0.0f;
    if (is_hovered && mouse_move)
      ui_c.grid_idx = i;

    ImGui::SetCursorScreenPos(ImVec2{ x_pct, y_pct });
    const auto& cell = ui_c.state.cells[i];
    const auto name = cell->name;
    const auto [aquired, total] = get_upgrade_level(r, upgrade_c, name);
    ImGui::TextColored(im_text_col, "%i/%i", aquired, total);
  }

  ImGui::SetCursorScreenPos(ImVec2{ grid_tl.x, grid_br.y });
  ImGui::PushStyleColor(ImGuiCol_Separator, im_separator_col);
  ImGui::Separator();
  ImGui::PopStyleColor();

  if (ui_c.selected_stat.has_value()) {
    const auto stat_str = std::string(magic_enum::enum_name<UpgradeableStat>(ui_c.selected_stat.value()));

    const auto find_by_key = [&stat_str](Upgrade& u) { return u.key == stat_str; };
    const auto it = std::find_if(upgrade_c.upgrades.begin(), upgrade_c.upgrades.end(), find_by_key);
    if (it == upgrade_c.upgrades.end()) {
      const auto err = std::format("Upgrade does not exist: {}", stat_str);
      throw std::runtime_error(err.c_str());
    }
    const Upgrade u = (*it);

    const auto [aquired, total] = get_upgrade_level(r, upgrade_c, stat_str);
    ImGui::TextColored(im_text_col, "Upgrade: %s. Available: %i. Purchased: %i.", u.key.c_str(), total, aquired);

    // loaded on-disk values
    int your_level = 0;
    auto str_opt = savefile_get_key(r, stat_str);
    if (str_opt.has_value())
      str_opt->get_to(your_level);

    // display the current upgrade level, and future upgrade levels
    for (int i = 0; i < u.levels.size(); i++) {
      const UpgradeLevel& l = u.levels[i];
      const bool aquired = i < your_level;
      ;

      std::string str = "";
      if (l.type == "stat_percent_increase")
        str = std::format("({}) {}G. +{}%", i + 1, l.cost, l.value, aquired);
      else if (l.type == "stat_flat_increase")
        str = std::format("({}) {}G. +{}", i + 1, l.cost, l.value, aquired);

      if (aquired)
        ImGui::TextColored(aquired_col, "%s", str.c_str());
      else
        ImGui::TextColored(unaquired_col, "%s", str.c_str());
    }
  }

  ImGui::PopFont();
  ImGui::End();
}

} // namespace game2d