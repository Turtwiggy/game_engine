#include "grid.hpp"
#include "modules/grid/components.hpp"

namespace engine {

namespace grid {

std::vector<std::pair<GridDirection, int>>
get_neighbour_indicies(const int x, const int y, const int x_max, const int y_max)
{
  const int max_idx = x_max * y_max;
  const int idx_north = x_max * (y - 1) + x;
  const int idx_east = x_max * y + (x + 1);
  const int idx_south = x_max * (y + 1) + x;
  const int idx_west = x_max * y + (x - 1);
  const bool ignore_north = y <= 0;
  const bool ignore_east = x >= x_max - 1;
  const bool ignore_south = y >= y_max - 1;
  const bool ignore_west = x <= 0;

  std::vector<std::pair<GridDirection, int>> results;

  if (!ignore_north && idx_north >= 0 && idx_north < max_idx)
    results.push_back({ GridDirection::north, idx_north });

  if (!ignore_east && idx_east >= 0 && idx_east < max_idx)
    results.push_back({ GridDirection::east, idx_east });

  if (!ignore_south && idx_south >= 0 && idx_south < max_idx)
    results.push_back({ GridDirection::south, idx_south });

  if (!ignore_west && idx_west >= 0 && idx_west < max_idx)
    results.push_back({ GridDirection::west, idx_west });

  return results;
};

std::vector<std::pair<GridDirection, glm::ivec2>>
get_neighbour_gridpos(const glm::ivec2& xy, const int x_max, const int y_max)
{
  std::vector<std::pair<GridDirection, glm::ivec2>> results;
  results.push_back({ GridDirection::north, { xy.x, xy.y + 1 } });
  results.push_back({ GridDirection::east, { xy.x + 1, xy.y } });
  results.push_back({ GridDirection::south, { xy.x, xy.y - 1 } });
  results.push_back({ GridDirection::west, { xy.x - 1, xy.y } });
  return results;
};

std::vector<std::pair<GridDirection, int>>
get_neighbour_indicies_with_diagonals(const int x, const int y, const int x_max, const int y_max)
{
  const int max_idx = x_max * y_max;
  const int idx_north = x_max * (y - 1) + x;
  const int idx_east = x_max * y + (x + 1);
  const int idx_south = x_max * (y + 1) + x;
  const int idx_west = x_max * y + (x - 1);
  const int idx_north_east = x_max * (y - 1) + (x + 1);
  const int idx_south_east = x_max * (y + 1) + (x + 1);
  const int idx_south_west = x_max * (y + 1) + (x - 1);
  const int idx_north_west = x_max * (y - 1) + (x - 1);
  const bool ignore_north = y <= 0;
  const bool ignore_east = x >= x_max - 1;
  const bool ignore_south = y >= y_max - 1;
  const bool ignore_west = x <= 0;
  const bool ignore_north_east = ignore_north | ignore_east;
  const bool ignore_south_east = ignore_south | ignore_east;
  const bool ignore_south_west = ignore_south | ignore_west;
  const bool ignore_north_west = ignore_north | ignore_west;

  std::vector<std::pair<GridDirection, int>> results;

  if (!ignore_north && idx_north >= 0 && idx_north < max_idx)
    results.push_back({ GridDirection::north, idx_north });

  if (!ignore_east && idx_east >= 0 && idx_east < max_idx)
    results.push_back({ GridDirection::east, idx_east });

  if (!ignore_south && idx_south >= 0 && idx_south < max_idx)
    results.push_back({ GridDirection::south, idx_south });

  if (!ignore_west && idx_west >= 0 && idx_west < max_idx)
    results.push_back({ GridDirection::west, idx_west });

  if (!ignore_north_east && idx_north_east >= 0 && idx_north_east < max_idx)
    results.push_back({ GridDirection::north_east, idx_north_east });

  if (!ignore_south_east && idx_south_east >= 0 && idx_south_east < max_idx)
    results.push_back({ GridDirection::south_east, idx_south_east });

  if (!ignore_south_west && idx_south_west >= 0 && idx_south_west < max_idx)
    results.push_back({ GridDirection::south_west, idx_south_west });

  if (!ignore_north_west && idx_north_west >= 0 && idx_north_west < max_idx)
    results.push_back({ GridDirection::north_west, idx_north_west });

  return results;
};

glm::ivec2
index_to_grid_position(const int index, const int x_max, const int y_max)
{
  const int x = index % x_max;
  const int y = static_cast<int>(index / static_cast<float>(y_max));
  return { x, y };
};

engine::grid::GridDirection
which_quadrant_is_b(const glm::ivec2& a, const glm::ivec2& b)
{
  const auto dir = b - a;
  const bool on_diagonal = glm::abs(dir.x == dir.y);
  const bool north = dir.y < 0;
  const bool south = dir.y > 0;
  const bool east = dir.x < 0;
  const bool west = dir.x > 0;
  const bool y_pull = glm::abs(dir.y) > glm::abs(dir.x);
  const bool x_pull = glm::abs(dir.x) > glm::abs(dir.y);

  const bool in_quad_N = y_pull && north;
  const bool in_quad_S = y_pull && south;
  const bool in_quad_E = x_pull && east;
  const bool in_quad_W = x_pull && west;
  const bool in_quad_NE = on_diagonal && north && east;
  const bool in_quad_NW = on_diagonal && north && west;
  const bool in_quad_SE = on_diagonal && south && east;
  const bool in_quad_SW = on_diagonal && south && west;

  if (in_quad_N)
    return engine::grid::GridDirection::north;
  if (in_quad_S)
    return engine::grid::GridDirection::south;
  if (in_quad_E)
    return engine::grid::GridDirection::east;
  if (in_quad_W)
    return engine::grid::GridDirection::west;

  if (in_quad_NE)
    return engine::grid::GridDirection::north_east;
  if (in_quad_NW)
    return engine::grid::GridDirection::north_west;
  if (in_quad_SE)
    return engine::grid::GridDirection::south_east;
  if (in_quad_SW)
    return engine::grid::GridDirection::south_west;

  return engine::grid::GridDirection::east;
};

bool
dir_is_in_quadrant(const glm::ivec2& dir, const engine::grid::GridDirection& direction)
{
  return which_quadrant_is_b({ 0, 0 }, dir) == direction;
};

bool
grid_position_in_bounds(const glm::ivec2& gp, const int x_max, const int y_max)
{
  if (gp.x < 0)
    return false;
  if (gp.x > x_max - 1)
    return false;
  if (gp.y < 0)
    return false;
  if (gp.y > y_max - 1)
    return false;

  return true;
};

} // namespace grid

} // namespace engine