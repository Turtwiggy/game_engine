#pragma once

#include "engine/maths/grid.hpp"
#include "game/entities/actors.hpp"

#include <entt/entt.hpp>

#include <map>

namespace game2d {

using Coord = std::pair<int, int>;
using Entities = std::vector<entt::entity>;

struct Dungeon
{
  int width = 30;
  int height = 30;

  std::map<Coord, Entities> tilemap;
};

struct Room
{
  int x = 0;
  int y = 0;
  int w = 0;
  int h = 0;
};

// If the room generation algorithm hits "step" rooms
// the algorithm is stopped. This can be useful for debugging
// by pausing it at "step" 1, 2, 3, 4 and seeing what the
// rooms look like at that "step".
void
generate_dungeon(entt::registry& r, Dungeon& d, int step = -1);

} // namespace game2d