#include "pch.hpp"

#include "ui_scene_upgrades_helpers.hpp"

#include "engine/colour/colour.hpp"
#include "engine/entt/helpers.hpp"
#include "modules/actors/actor_weapon/weapon_components.hpp"
#include "modules/core/fonts/fonts_helpers.hpp"
#include "modules/core/io/io_helpers.hpp"
#include "modules/systems/system_hardpoint_arcs/hulls_components.hpp"
#include "modules/systems/system_item_gold/gold_components.hpp"
#include "modules/systems/system_persistent_upgrades/persistent_upgrade_components.hpp"
#include "modules/systems/system_shop/shop_components.hpp"
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

// void
// process_input_for_grid(entt::registry& r, SINGLE_PersistentUpgradesMenuUI& ui_c)
// {
//   auto& acts = ui_c.state.actions;
//   const auto val_u = std::find(acts.begin(), acts.end(), UIAction::NAV_MOVE_U) != acts.end();
//   const auto val_d = std::find(acts.begin(), acts.end(), UIAction::NAV_MOVE_D) != acts.end();
//   const auto val_l = std::find(acts.begin(), acts.end(), UIAction::NAV_MOVE_L) != acts.end();
//   const auto val_r = std::find(acts.begin(), acts.end(), UIAction::NAV_MOVE_R) != acts.end();

//   const auto [cur_x, cur_y] = engine::grid::index_to_grid_position(ui_c.grid_idx, ui_c.grid_x);
//   auto new_x = cur_x;
//   auto new_y = cur_y;

//   if (val_r)
//     new_x++;
//   if (val_l)
//     new_x--;
//   if (val_u)
//     new_y--;
//   if (val_d)
//     new_y++;

//   int max_x = ui_c.grid_x - 1;
//   int max_y = get_grid_y(ui_c.state.cells.size(), ui_c.grid_x) - 1;
//   new_x = glm::clamp(new_x, 0, max_x);
//   new_y = glm::clamp(new_y, 0, max_y);
//   ui_c.grid_idx = engine::grid::grid_position_to_index({ new_x, new_y }, ui_c.grid_x);
//   ui_c.grid_idx = glm::clamp(ui_c.grid_idx, 0, (int)ui_c.state.cells.size() - 1);
// };

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

std::string
get_gridcell_item_key(entt::registry& r, const GridCell* gc)
{
  std::string key = "";

  if (gc->type == GridCellType::HULL) {
    const auto& hulls_c = get_first_component<SINGLE_Hulls>(r);
    const auto& hull = hulls_c.hulls[gc->index];
    key = hull.key;
  }

  if (gc->type == GridCellType::WEAPON) {
    const auto& weapons_c = get_first_component<SINGLE_Weapons>(r);
    const auto& weapon = weapons_c.weapons[gc->index];
    key = weapon.key;
  }

  if (gc->type == GridCellType::STAT) {
    const auto stat_enum = magic_enum::enum_cast<UpgradeableStat>(gc->index).value();
    const auto stat_str = std::string(magic_enum::enum_name<UpgradeableStat>(stat_enum));
    key = stat_str;
  }

  return key;
};

int
get_item_key_cost(entt::registry& r, std::string key)
{
  const auto& shop_c = get_first_component<SINGLE_Shop>(r);
  const auto cmp = [key](const Item_OnDiskData& a) { return a.key == key; };
  const auto it = std::find_if(shop_c.items.begin(), shop_c.items.end(), cmp);
  if (it == shop_c.items.end()) {
    throw std::runtime_error("scotty we have a problem; the item doesnt exist in the shop");
    return INT_MAX;
  }

  return it->cost;
};

} // namespace game2d