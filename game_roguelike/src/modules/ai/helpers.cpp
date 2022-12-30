#include "helpers.hpp"

#include "components.hpp"
#include "components/app.hpp"

namespace game2d {

void
game2d::get_neighbour_indicies(const int x,
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
game2d::get_neighbour_indicies_with_diagonals(const int x,
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

std::vector<vec2i>
astar(entt::registry& registry, const vec2i& from, const vec2i& to)
{
  // hack: only one dungeon at the moment
  const auto d = registry.view<Dungeon>().front();
  const auto& dungeon = registry.get<Dungeon>(d);
  const int x_max = dungeon.width;

  if (equal<vec2i>(from, to))
    return {};

  PriorityQueue<vec2i> frontier;
  frontier.enqueue(from, 0);
  std::map<vec2i, vec2i> came_from;
  std::map<vec2i, int> cost_so_far;
  came_from[from] = from;
  cost_so_far[from] = 0;

  // read-only view of the grid
  const auto& group = dungeon.walls_and_floors;

  while (frontier.size() > 0) {
    vec2i current = frontier.dequeue();

    if (equal<vec2i>(current, to))
      return reconstruct_path<vec2i>(came_from, from, to);

    std::vector<std::pair<GridDirection, int>> results;
    get_neighbour_indicies(current.x, current.y, dungeon.width, dungeon.height, results);

    for (const auto& neighbour_idx : results) {
      const auto& entity = group[neighbour_idx.second];
      const PathfindableComponent& path = registry.get<PathfindableComponent>(entity.entity);

      const vec2i neighbour = { entity.x, entity.y };
      const int neighbour_cost = path.cost;
      evaluate_neighbour<vec2i>(came_from, cost_so_far, frontier, current, neighbour, neighbour_cost, to);
    }
  }

  return {};
};

} // namespace game2d