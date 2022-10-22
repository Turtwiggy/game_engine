#pragma once

#include <glm/glm.hpp>
#include <nlohmann/json.hpp>

#include <vector>

namespace game2d {

struct Dungeon
{
  int width = 50;
  int height = 50;
  int seed = 0;
  int floor = 0;
};

struct Room
{
  int x1 = 0;
  int y1 = 0;
  int x2 = 0;
  int y2 = 0;
  int w = 0;
  int h = 0;

  std::vector<glm::ivec2> occupied;
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

struct GridComponent
{
  int x = 0;
  int y = 0;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(GridComponent, x, y);
};

} // namespace game2d