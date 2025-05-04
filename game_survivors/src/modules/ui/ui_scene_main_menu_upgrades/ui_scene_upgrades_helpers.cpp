#include "pch.hpp"

#include "ui_scene_upgrades_helpers.hpp"

namespace game2d {

int
get_grid_y(int n_cells, int grid_x)
{
  return (n_cells / grid_x) + 1;
}

} // namespace game2d