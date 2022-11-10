#pragma once

#include "game/components/app.hpp"
#include "game/modules/dungeon/components.hpp"

#include <entt/entt.hpp>
#include <glm/glm.hpp>

#include <vector>

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
};

void
mark_visible(Game& game, const entt::entity& e, int x, int y);

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

void
do_symmetric_shadowcasting(Game& game, const glm::ivec2 origin);

} // namespace game2d