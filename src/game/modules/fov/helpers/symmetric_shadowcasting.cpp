#include "symmetric_shadowcasting.hpp"

#include "game/components/actors.hpp"
#include "game/modules/fov/components.hpp"

#include <optional>
#include <vector>

namespace game2d {

void
mark_visible(Game& game, const entt::entity& e, int x, int y)
{
  if (e == entt::null)
    return;
  auto& r = game.state;
  if (auto* not_visible = r.try_get<NotVisibleComponent>(e))
    r.remove<NotVisibleComponent>(e);
  if (auto* was_visible = r.try_get<NotVisibleButPreviouslySeenComponent>(e))
    r.remove<NotVisibleButPreviouslySeenComponent>(e);

  VisibleComponent vc;
  vc.grid_x = x;
  vc.grid_y = y;
  r.emplace_or_replace<VisibleComponent>(e, vc);
};

bool
is_symmetric(const Row& row, const Tile& tile)
{
  int row_depth = tile.depth;
  int col = tile.col;
  bool result = (col >= row.depth * row.start_slope) && (col <= row.depth * row.end_slope);
  // printf(" checking is_symmetric: %i \n", result);
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

void
do_symmetric_shadowcasting(Game& game, const glm::ivec2 origin)
{
  auto& r = game.state;

  // hack: only one dungeon at the moment
  const auto d = r.view<Dungeon>().front();
  const auto& dungeon = r.get<Dungeon>(d);
  const int x_max = dungeon.width;

  // read-only view of the grid
  const std::vector<StaticDungeonEntity>& group = dungeon.walls_and_floors;

  // mark origin visible()
  {
    const int index = x_max * origin.y + origin.x;
    const auto& tile = group[index];
    mark_visible(game, tile.entity, origin.x, origin.y);
  }

  // check all directions
  for (int dir = 0; dir < 4; dir++) {

    auto transform = [&dir, &origin](const Tile t) -> std::pair<int, int> {
      int row = t.depth;
      int col = t.col;
      if (dir == 0) // north
        return { origin.x + col, origin.y - row };
      if (dir == 2) // south
        return { origin.x + col, origin.y + row };
      if (dir == 1) // east
        return { origin.x + row, origin.y + col };
      if (dir == 3) // west
        return { origin.x - row, origin.y + col };
      exit(0); // crash for now
    };

    auto is_wall = [&transform, &x_max, &r, &group](const Tile t) {
      const auto [x, y] = transform(t);
      int index = x_max * y + x;
      entt::entity e = group[index].entity;
      const auto& type = r.get<EntityTypeComponent>(e);
      bool result = type.type == EntityType::tile_type_wall;
      // printf("checking wall: %i %i. was_wall: %i \n", x, y, result);
      return result;
    };

    auto is_floor = [&transform, &x_max, &r, &group](const Tile t) {
      const auto [x, y] = transform(t);
      int index = x_max * y + x;
      entt::entity e = group[index].entity;
      const auto& type = r.get<EntityTypeComponent>(e);
      bool result = type.type == EntityType::tile_type_floor;
      // printf(" checking floor: %i %i. was_floor: %i \n", x, y, result);
      return result;
    };

    Row first_row;
    first_row.depth = 1;
    first_row.start_slope = -1.0f;
    first_row.end_slope = 1.0f;

    std::vector<Row> rows;
    rows.push_back(first_row);

    while (!rows.empty()) {
      Row row = rows.back();
      rows.pop_back();
      std::optional<Tile> prev_tile;

      int min_col = round_up(row.depth * row.start_slope);
      int max_col = round_down(row.depth * row.end_slope);
      // printf("min col: %i max_col %i start_slope: %f end_slope: %f", min_col, max_col, row.start_slope,
      // row.end_slope);

      for (int i = min_col; i < (max_col + 1); i++) { // tiles()

        Tile tile;
        tile.depth = row.depth;
        tile.col = i;
        // printf("iterating new tile: %i %i \n", tile.depth, tile.col);

        if (is_wall(tile) || is_symmetric(row, tile)) {
          // reveal();
          const auto [x, y] = transform(tile);
          // printf("revealing %i %i \n", x, y);
          const int index = x_max * y + x;
          const auto& se = group[index];
          mark_visible(game, se.entity, x, y);
        }

        if ((prev_tile.has_value() && is_wall(prev_tile.value())) && is_floor(tile)) {
          row.start_slope = slope(tile);
          // printf("slope recalculated: %f \n", row.start_slope);
        }

        if ((prev_tile.has_value() && is_floor(prev_tile.value()) && is_wall(tile))) {
          Row next_row = next(row);
          next_row.end_slope = slope(tile);
          rows.push_back(next_row);
          // printf("new row pushed back! depth: %i end_slope: %f", next_row.depth, next_row.end_slope);
        }

        // printf("prev_tile assigned to\n");
        prev_tile = tile;
      }

      if (prev_tile.has_value() && is_floor(prev_tile.value())) {
        // printf("next row pushed back\n");
        rows.push_back(next(row));
      }
    }
  }
}

} // namespace game2d