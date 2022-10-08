#pragma once

#include "engine/maths/grid.hpp"
#include "game/components/app.hpp"
#include "game/entities/actors.hpp"

#include <entt/entt.hpp>

#include <map>
#include <vector>

namespace game2d {

struct Dungeon
{
  int width = 50;
  int height = 50;
};

struct Room
{
  int x1 = 0;
  int y1 = 0;
  int x2 = 0;
  int y2 = 0;
  int w = 0;
  int h = 0;
};

enum class GridDirection : size_t
{
  north,
  south,
  east,
  west,

  // allow diagonals?
  // north_east,
  // south_east,
  // south_west,
  // north_west,

  count,
};

std::vector<entt::entity>
grid_entities_at(entt::registry& r, int x, int y);

// e.g. 0, 0 should return {1, 0}, {1, 1}, {0, 1}
void
get_neighbour_indicies(const int x,
                       const int y,
                       const int x_max,
                       const int y_max,
                       std::vector<std::pair<GridDirection, int>>& results);

// If the room generation algorithm hits "step" rooms
// the algorithm is stopped. This can be useful for debugging
// by pausing it at "step" 1, 2, 3, 4 and seeing what the
// rooms look like at that "step".
void
generate_dungeon(GameEditor& editor, Game& game, const Dungeon& d);

void
update_dungeon_system(GameEditor& editor, Game& game);

} // namespace game2d