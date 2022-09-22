#pragma once

#include "engine/maths/grid.hpp"
#include "game/entities/actors.hpp"

#include <entt/entt.hpp>

#include <map>

namespace game2d {

struct Dungeon
{
  int width = 30;
  int height = 30;
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

enum class GridDirection
{
  NORTH,
  EAST,
  SOUTH,
  WEST,

  // DIAGONALS?
  NORTH_EAST,
  SOUTH_EAST,
  NORTH_WEST,
  SOUTH_WEST,

  COUNT
};

struct GridTileComponent
{
  int x = 0;
  int y = 0;

  // pathfinding
  int cost = 0;

  // spaceship operator
  auto operator<=>(const GridTileComponent&) const = default;
};

std::vector<entt::entity>
grid_entities_at(entt::registry& r, int x, int y);

// If the room generation algorithm hits "step" rooms
// the algorithm is stopped. This can be useful for debugging
// by pausing it at "step" 1, 2, 3, 4 and seeing what the
// rooms look like at that "step".
void
generate_dungeon(entt::registry& r, const Dungeon& d, int step = -1);

void
update_dungeon_system(entt::registry& r);

} // namespace game2d