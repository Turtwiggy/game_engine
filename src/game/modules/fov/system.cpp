#include "system.hpp"

#include "components.hpp"
#include "engine/maths/grid.hpp"
#include "game/components/actors.hpp"
#include "game/modules/ai/components.hpp"
#include "game/modules/combat/components.hpp"
#include "game/modules/dungeon/components.hpp"
#include "game/modules/dungeon/helpers.hpp"
#include "game/modules/fov/helpers.hpp"
#include "game/modules/player/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/sprites/components.hpp"
#include "resources/colour.hpp"

#include <glm/glm.hpp>

#include <optional>
#include <queue>
#include <vector>

namespace game2d {

void
init_tile_fov_system(GameEditor& editor, Game& game)
{
  auto& r = game.state;
  const auto d = r.view<Dungeon>().front();
  const auto& dungeon = r.get<Dungeon>(d);

  // read-only view of the grid
  const std::vector<StaticDungeonEntity>& group = dungeon.walls_and_floors;

  // Set everything as hidden
  for (const auto& se : group)
    r.emplace<NotVisibleComponent>(se.entity);
}

void
update_tile_fov_system(GameEditor& editor, Game& game)
{
  const auto& colours = editor.colours;
  auto& r = game.state;

  // hack: only one dungeon at the moment
  const auto d = r.view<Dungeon>().front();
  const auto& dungeon = r.get<Dungeon>(d);
  const int x_max = dungeon.width;

  const auto player_view = r.view<PlayerComponent>();
  if (player_view.size() == 0)
    return;
  const auto player_entity = player_view.front();
  const auto& player_transform = r.get<TransformComponent>(player_entity);
  glm::ivec2 origin =
    engine::grid::world_space_to_grid_space({ player_transform.position.x, player_transform.position.y }, 16);

  // read-only view of the grid
  const std::vector<StaticDungeonEntity>& group = dungeon.walls_and_floors;

  r.clear<VisibleComponent>();
  r.clear<NotVisibleComponent>();
  for (const auto& tile : group)
    r.emplace<NotVisibleComponent>(tile.entity);

  // mark origin visible()
  {
    const int index = x_max * origin.y + origin.x;
    const auto& tile = group[index];
    r.emplace_or_replace<VisibleComponent>(tile.entity);
    if (auto* was_hidden = r.try_get<NotVisibleComponent>(tile.entity))
      r.remove<NotVisibleComponent>(tile.entity);
  }

  // printf("player is at: %i %i \n", origin.x, origin.y);

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
          r.emplace_or_replace<VisibleComponent>(se.entity);
          if (auto* was_hidden = r.try_get<NotVisibleComponent>(se.entity))
            r.remove<NotVisibleComponent>(se.entity);
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
  // printf("END\n");

  //
  // set colours
  //

  // visible
  {
    auto visible_no_hp_view = r.view<const VisibleComponent, SpriteColourComponent, const EntityTypeComponent>();
    visible_no_hp_view.each(
      [&r, &colours](const VisibleComponent& v, SpriteColourComponent& scc, const EntityTypeComponent& et) {
        //
        scc.colour = colours.lin_red;
      });
  }

  // not visible
  {
    auto not_visible_view = r.view<const NotVisibleComponent, SpriteColourComponent>();
    not_visible_view.each([&colours](const NotVisibleComponent& v, SpriteColourComponent& scc) {
      //
      scc.colour = colours.lin_black;
    });
  }

  // was visible
  {
    auto was_visible_view = r.view<const NotVisibleButPreviouslySeenComponent, SpriteColourComponent>();
    was_visible_view.each([&colours](const NotVisibleButPreviouslySeenComponent& v, SpriteColourComponent& scc) {
      //
      scc.colour = colours.lin_feint_white;
    });
  }
}

} // namespace game2d