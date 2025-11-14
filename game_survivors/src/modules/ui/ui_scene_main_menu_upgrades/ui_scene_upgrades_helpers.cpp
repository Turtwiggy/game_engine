#include "pch.hpp"

#include "ui_scene_upgrades_helpers.hpp"

#include "engine/colour/colour.hpp"
#include "engine/entt/helpers.hpp"
#include "modules/core/fonts/fonts_helpers.hpp"
#include "modules/core/io/io_helpers.hpp"
#include "modules/systems/system_item_gold/gold_components.hpp"
#include "modules/systems/system_persistent_upgrades/persistent_upgrade_components.hpp"
#include "modules/ui/ui_scene_main_menu/ui_scene_main_menu_components.hpp"
#include "modules/ui/ui_scene_main_menu_upgrades/ui_scene_upgrades_components.hpp"
#include "resources/data.hpp"

namespace game2d {

int
get_grid_y(int n_cells, int grid_x)
{
  return (n_cells / grid_x) + 1;
}

void
back_to_main_menu(entt::registry& r, SINGLE_PersistentUpgradesMenuUI& ui_c)
{
  ui_c.open = false;
  ui_c.one_frame_buffer = true;
  ui_c.state.active = nullptr;
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
    // throw std::runtime_error(err.c_str());
    SDL_Log(err.c_str());
    return;
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
    if (!u.levels.empty())
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

  // Prefix the button with the your_aquired out of available_aquired
  // display_stat_key
  // std::string button_str = std::format("{}/{}", n_stat_upgrades_aquired, n_stat_upgrades);
  return { n_stat_upgrades_aquired, n_stat_upgrades };
};

void
draw_purchasebar(entt::registry& r,
                 const ImVec2 tl,
                 const ImVec2 br,
                 const float percent,
                 const std::string text,
                 const engine::SRGBColour& col)
{
  auto* draw_list = ImGui::GetWindowDrawList();
  auto* text_font = get_inter_font(r);
  const auto font_text_size = (float)FontSizes::SIZE_16;

  const auto purchasebar_wh = br - tl;

  auto my_player_col = col;
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

  draw_bar(tl, br, percent);

  const auto text_size = text_font->CalcTextSizeA(font_text_size, FLT_MAX, -1, text.c_str());
  const auto text_pos =
    ImVec2{ tl.x + 0.5f * (purchasebar_wh.x - text_size.x), tl.y + 0.5f * (purchasebar_wh.y - text_size.y) };
  draw_list->AddText(text_font, font_text_size, text_pos, im_text_col, text.c_str());
}

} // namespace game2d