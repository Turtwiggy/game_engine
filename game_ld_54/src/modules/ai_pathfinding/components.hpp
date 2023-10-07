#pragma once

namespace game2d {

struct PathfindComponent
{
  int cost = 1;
};

struct vec2i
{
  int x = 0;
  int y = 0;

  // spaceship operator
  auto operator<=>(const vec2i&) const = default;
};

struct astar_cell
{
  int idx = 0;
  vec2i pos;
  int distance = INT_MAX;
  int cost = 1;
};

} // namespace game2d