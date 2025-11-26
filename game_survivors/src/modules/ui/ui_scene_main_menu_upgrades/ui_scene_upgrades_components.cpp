#include "pch.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/maths/grid.hpp"
#include "engine/maths/maths.hpp"
#include "modules/actors/actor_weapon/weapon_components.hpp"
#include "modules/core/io/io_helpers.hpp"
#include "modules/core/ui/ui_common_helpers.hpp"
#include "modules/systems/system_hardpoint_arcs/hulls_components.hpp"
#include "modules/systems/system_persistent_upgrades/persistent_upgrade_components.hpp"
#include "modules/ui/ui_popup_options/ui_popup_options_components.hpp"
#include "modules/ui/ui_scene_survive_upgrade/ui_survive_upgrade_helpers.hpp"
#include "ui_scene_upgrades_components.hpp"
#include "ui_scene_upgrades_helpers.hpp"

namespace game2d {
using namespace engine::grid;

// void
// create_as_grid_layout(std::vector<std::shared_ptr<Cell>>& cells)
// {
/*
// setup navigation
 {
   const int s = state.cells.size();
   const int grid_y = get_grid_y(s, grid_x);

   // setup horizontal navigation for rows
   // i.e. connect left and right.

   for (int x = 0; x < grid_x; x++) {
     for (int y = 0; y < grid_y; y++) {
       const int idx = grid_position_to_index({ x, y }, grid_x);
       if (idx >= state.cells.size())
         break; // no more cells
       const auto cell = state.cells[idx];

       const auto l_idx = grid_position_to_index({ engine::wrap(x - 1, grid_x - 1), y }, grid_x + 1);
       const auto r_idx = grid_position_to_index({ engine::wrap(x + 1, grid_x - 1), y }, grid_x + 1);
       const auto u_idx = grid_position_to_index({ x, engine::wrap(y - 1, grid_y) }, grid_x + 1);
       const auto d_idx = grid_position_to_index({ x, engine::wrap(y + 1, grid_y) }, grid_x + 1);

       // connect in all directions
       // clang-format off
       const auto valid = [&s](int idx) { return idx >= 0 && idx <= (s - 1); };
       if (valid(l_idx)) cell->l = state.cells[l_idx];
       if (valid(r_idx)) cell->r = state.cells[r_idx];
       if (valid(u_idx)) cell->u = state.cells[u_idx];
       if (valid(d_idx)) cell->d = state.cells[d_idx];
       // clang-format on
     }
   }
 }
*/
// }

std::vector<std::shared_ptr<Cell>>
get_boats(entt::registry& r)
{
  const auto data_e = get_first<SINGLE_Hulls>(r);
  if (data_e == entt::null)
    return {};
  auto& data_c = r.get<SINGLE_Hulls>(data_e);

  std::vector<std::shared_ptr<Cell>> cells;

  for (int i = 0; i < data_c.hulls.size(); i++) {
    const auto& hull = data_c.hulls[i];
    GridCell c(GridCellType::HULL, i);
    c.name = hull.name.c_str();
    c.action = [&]() {};
    cells.push_back(std::make_shared<GridCell>(c));
  }

  create_as_horizontal_layout(cells);
  return cells;
}

std::vector<std::shared_ptr<Cell>>
get_weapons(entt::registry& r)
{
  const auto weapons_e = get_first<SINGLE_Weapons>(r);
  if (weapons_e == entt::null)
    return {};
  auto& weapons_c = r.get<SINGLE_Weapons>(weapons_e);

  std::vector<std::shared_ptr<Cell>> cells;

  for (int i = 0; i < weapons_c.weapons.size(); i++) {
    const auto& weapon = weapons_c.weapons[i];
    if (weapon.useable_by_as_enum != WEAPON_USEABLE_BY::BOATS)
      continue;
    GridCell c(GridCellType::WEAPON, i);
    c.name = weapon.name.c_str();
    c.action = [&]() {};
    cells.push_back(std::make_shared<GridCell>(c));
  }

  create_as_horizontal_layout(cells);
  return cells;
}

std::vector<std::shared_ptr<Cell>>
get_stats(entt::registry& r)
{
  const auto upgrade_e = get_first<SINGLE_PersistentUpgrades>(r);
  if (upgrade_e == entt::null)
    return {};
  auto& upgrade_c = r.get<SINGLE_PersistentUpgrades>(upgrade_e);

  std::vector<std::shared_ptr<Cell>> cells;

  for (int i = 0; i < (int)UpgradeableStat::count; i++) {
    const auto stat_enum = magic_enum::enum_value<UpgradeableStat>(i);
    const auto stat_str = std::string(magic_enum::enum_name<UpgradeableStat>(stat_enum));
    // stats to skip...
    if (stat_enum == UpgradeableStat::ACTOR_STAMINA)
      continue;
    // if there are no persistent upgrades, skip the stat.
    const auto [aquired, total] = get_upgrade_level(r, upgrade_c, stat_str);
    if (total == 0)
      continue;

    const auto pretty_str = make_stat_name_pretty_name(stat_str);

    GridCell c(GridCellType::STAT, i);
    c.name = pretty_str.c_str();
    c.action = [&]() {};
    cells.push_back(std::make_shared<GridCell>(c));
  }

  create_as_horizontal_layout(cells);

  return cells;
};

void
SINGLE_PersistentUpgradesMenuUI::do_init(entt::registry& r)
{
  auto& cells = state.cells;

  HeaderCell oc;
  oc.name = "Boats";
  oc.value = 0;
  oc.gridcells = get_boats(r);
  cells.push_back(std::make_shared<HeaderCell>(oc));

  HeaderCell oc1;
  oc1.name = "Weapons";
  oc1.value = 0;
  oc1.gridcells = get_weapons(r);
  cells.push_back(std::make_shared<HeaderCell>(oc1));

  HeaderCell oc2;
  oc2.name = "Crew";
  oc2.value = 0;
  oc2.gridcells = get_stats(r);
  cells.push_back(std::make_shared<HeaderCell>(oc2));

  create_as_horizontal_layout(state.cells);

  // link header to cells

  auto link = [](auto hmm) {
    auto& gridcells = ((HeaderCell*)hmm.get())->gridcells;

    // up goes to the header
    for (auto& gridcell : gridcells)
      gridcell->u = hmm;

    // down goes to the first cell
    hmm->d = gridcells[0];

    //
  };
  link(cells[0]);
  link(cells[1]);
  link(cells[2]);

  state.active = state.cells[0];
  active_header = state.cells[0];
  init = true;
}

} // namespace game2d