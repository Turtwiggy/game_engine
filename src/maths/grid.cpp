#include "grid.hpp"

namespace engine {

namespace grid {

void
get_neighbour_indicies(const int x,
                       const int y,
                       const int x_max,
                       const int y_max,
                       std::vector<std::pair<GridDirection, int>>& results)
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

  if (!ignore_north && idx_north >= 0 && idx_north < max_idx)
    results.push_back({ GridDirection::north, idx_north });

  if (!ignore_east && idx_east >= 0 && idx_east < max_idx)
    results.push_back({ GridDirection::east, idx_east });

  if (!ignore_south && idx_south >= 0 && idx_south < max_idx)
    results.push_back({ GridDirection::south, idx_south });

  if (!ignore_west && idx_west >= 0 && idx_west < max_idx)
    results.push_back({ GridDirection::west, idx_west });
};

void
get_neighbour_indicies_with_diagonals(const int x,
                                      const int y,
                                      const int x_max,
                                      const int y_max,
                                      std::vector<std::pair<GridDirection, int>>& results)
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
};

}

} // namespace game2d