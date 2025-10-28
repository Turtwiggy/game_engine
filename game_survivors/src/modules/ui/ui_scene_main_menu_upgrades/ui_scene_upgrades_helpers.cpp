#include "pch.hpp"

#include "ui_scene_upgrades_helpers.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/maths/grid.hpp"
#include "modules/core/io/io_helpers.hpp"
#include "modules/systems/system_item_gold/gold_components.hpp"
#include "modules/systems/system_persistent_upgrades/persistent_upgrade_components.hpp"
#include "modules/ui/ui_scene_main_menu/ui_scene_main_menu_components.hpp"
#include "modules/ui/ui_scene_main_menu_upgrades/ui_scene_upgrades_components.hpp"

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
    new_y--;
  if (val_d)
    new_y++;

  int max_x = ui_c.grid_x - 1;
  int max_y = get_grid_y(ui_c.state.cells.size(), ui_c.grid_x) - 1;
  new_x = glm::clamp(new_x, 0, max_x);
  new_y = glm::clamp(new_y, 0, max_y);
  ui_c.grid_idx = engine::grid::grid_position_to_index({ new_x, new_y }, ui_c.grid_x);
  ui_c.grid_idx = glm::clamp(ui_c.grid_idx, 0, (int)ui_c.state.cells.size() - 1);
};

} // namespace game2d