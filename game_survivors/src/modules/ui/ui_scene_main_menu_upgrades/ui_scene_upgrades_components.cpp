#include "pch.hpp"

#include "engine/maths/grid.hpp"
#include "engine/maths/maths.hpp"
#include "ui_scene_upgrades_components.hpp"
#include "ui_scene_upgrades_helpers.hpp"

namespace game2d {
using namespace engine::grid;

void
SINGLE_PersistentUpgradesMenuUI::do_init(entt::registry& r)
{
  for (int i = 0; i < (int)UpgradeableStat::count; i++) {

    const auto stat_enum = magic_enum::enum_value<UpgradeableStat>(i);
    const auto stat_str = std::string(magic_enum::enum_name<UpgradeableStat>(stat_enum));

    // stats to skip...
    if (stat_str.find("ACTOR_STAMINA") != std::string::npos)
      continue;

    Cell c;
    c.name = stat_str.c_str();
    c.action = [&]() { selected_stat = stat_enum; };
    state.cells.push_back(std::make_shared<Cell>(c));
  }

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

        const auto l_idx = grid_position_to_index({ engine::wrap(x - 1, grid_x - 1), y }, grid_x);
        const auto r_idx = grid_position_to_index({ engine::wrap(x + 1, grid_x - 1), y }, grid_x);
        const auto u_idx = grid_position_to_index({ x, engine::wrap(y - 1, grid_y) }, grid_x);
        const auto d_idx = grid_position_to_index({ x, engine::wrap(y + 1, grid_y) }, grid_x);

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

  state.active = state.cells[0];
  init = true;
}

} // namespace game2d