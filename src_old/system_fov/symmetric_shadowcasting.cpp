#include "symmetric_shadowcasting.hpp"

#include "components.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/maths/grid.hpp"
#include "modules/grid/components.hpp"

namespace game2d {

void
mark_visible(entt::registry& r, const entt::entity& e)
{
  r.emplace_or_replace<SeenComponent>(e);
  r.emplace_or_replace<VisibleComponent>(e);
};

bool
is_symmetric(const Row& row, const Tile& tile)
{
  int row_depth = tile.depth;
  int col = tile.col;
  bool result = (col >= row.depth * row.start_slope) && (col <= row.depth * row.end_slope);
  return result;
};

Row
next(const Row& r)
{
  Row new_r;
  new_r.depth = r.depth + 1;
  new_r.start_slope = r.start_slope;
  new_r.end_slope = r.end_slope;
  return new_r;
};

float
slope(const Tile& t)
{
  int numerator = 2 * t.col - 1;
  float denominator = 2.0f * t.depth;
  float result = numerator / denominator;
  // printf("t: %i %i slope calc: %f \n", t.depth, t.col, result);
  return result;
};

int
round_up(const float n)
{
  return static_cast<int>(glm::floor(n + 0.5f));
};

int
round_down(const float n)
{
  return static_cast<int>(glm::ceil(n - 0.5f));
};

std::pair<int, int>
transform(const Tile t, const int dir, const glm::ivec2& origin)
{
  int row = t.depth;
  int col = t.col;
  if (dir == static_cast<uint32_t>(TileDir::North))
    return { origin.x + col, origin.y - row };
  if (dir == static_cast<uint32_t>(TileDir::South))
    return { origin.x + col, origin.y + row };
  if (dir == static_cast<uint32_t>(TileDir::East))
    return { origin.x + row, origin.y + col };
  if (dir == static_cast<uint32_t>(TileDir::West))
    return { origin.x - row, origin.y + col };
  exit(0); // crash for now
};

std::vector<int>
do_shadowcasting(entt::registry& r, const glm::ivec2& origin, const std::vector<int>& walls_or_floors)
{
  const auto& map = get_first_component<MapComponent>(r);
  std::vector<int> revealed_idxs;

  for (int dir = 0; dir < 4; dir++) {
    const auto is_type = [&](const Tile& t, const TileType& type) -> bool {
      const auto [x, y] = transform(t, dir, origin);
      const auto idx = engine::grid::grid_position_to_index({ x, y }, map.xmax);
      return walls_or_floors[idx] == static_cast<int>(type);
    };
    const auto is_wall = [&](const Tile& t) -> bool { return is_type(t, TileType::WALL); };
    const auto is_floor = [&](const Tile& t) -> bool { return is_type(t, TileType::FLOOR); };

    Row first_row;
    first_row.depth = 1;
    first_row.start_slope = -1.0f;
    first_row.end_slope = 1.0f;
    std::vector<Row> rows{ first_row };

    while (!rows.empty()) {
      Row row = rows.back();
      rows.pop_back();
      std::optional<Tile> prev_tile = std::nullopt;

      const int min_col = round_up(row.depth * row.start_slope);
      const int max_col = round_down(row.depth * row.end_slope);
      for (int i = min_col; i < (max_col + 1); i++) { // row.tiles()

        Tile tile;
        tile.depth = row.depth;
        tile.col = i;

        const auto [x, y] = transform(tile, dir, origin);
        const auto idx = engine::grid::grid_position_to_index({ x, y }, map.xmax);
        if (idx < 0)
          continue; // hmm...?

        // The current tile is a wall,
        // mark it as visible and move on
        if (is_wall(tile) || is_symmetric(row, tile)) {
          const auto [x, y] = transform(tile, dir, origin);
          const int idx = map.xmax * y + x;
          revealed_idxs.push_back(idx);
        }

        if (prev_tile.has_value() && is_wall(prev_tile.value()) && is_floor(tile))
          row.start_slope = slope(tile);

        if (prev_tile.has_value() && is_floor(prev_tile.value()) && is_wall(tile)) {
          Row next_row = next(row);
          next_row.end_slope = slope(tile);
          rows.push_back(next_row);
        }

        prev_tile = tile;
      }

      if (prev_tile.has_value() && is_floor(prev_tile.value())) {
        rows.push_back(next(row)); // scan(row.next())
      }
    }
  }

  return revealed_idxs;
}

} // namespace game2d