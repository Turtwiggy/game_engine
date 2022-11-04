#include "helpers.hpp"

#include "components.hpp"
#include "game/components/actors.hpp"
#include <glm/glm.hpp>

#include <functional>
#include <queue>
#include <vector>

namespace game2d {

void
mark_visible(Game& game, const entt::entity& e)
{
  if (e == entt::null)
    return;
  auto& r = game.state;
  if (auto* not_visible = r.try_get<NotVisibleComponent>(e))
    r.remove<NotVisibleComponent>(e);
  r.emplace_or_replace<VisibleComponent>(e);
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

} // namespace game2d