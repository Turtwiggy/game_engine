#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace game2d {

// inspired by:
// https://www.albertford.com/shadowcasting/
// https://github.com/370417/symmetric-shadowcasting/blob/master/shadowcasting.py

struct Row
{
  int depth = 0;
  float start_slope = 0.0f;
  float end_slope = 0.0f;
  int min_col = 0;
  int max_col = 0;
};

struct Tile
{
  int depth = 0;
  int col = 0;

  Tile() = default;
  Tile(int d, int c)
    : depth(d)
    , col(c){};
};

enum class TileType : uint32_t
{
  FLOOR = 0,
  WALL = 1,
};

enum class TileDir : uint32_t
{
  North = 0,
  East = 1,
  South = 2,
  West = 3,
};

// is_symmetric checks if a given floor tile
// can be seen symmetrically from the origin.
// It returns true if the central point of
// the tile is in the sector swept out by the
// row’s start and end slopes.
// Otherwise, it returns false.
bool
is_symmetric(const Row& row, const Tile& tile);

Row
next(const Row& r);

float
slope(const Tile& t);

int
round_up(const float n);

int
round_down(const float n);

std::pair<int, int>
transform(const Tile t, const int dir, const glm::ivec2& origin);

//

std::vector<int>
do_shadowcasting(entt::registry& r, const glm::ivec2& origin, const std::vector<int>& walls_or_floors);

void
mark_visible(entt::registry& r, const entt::entity& e);

} // namespace game2d