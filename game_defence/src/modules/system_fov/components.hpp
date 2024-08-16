#pragma once

#include <entt/entt.hpp>

namespace game2d {

struct VisibleComponent
{
  // cached values to avoid recomputing
  // only should check values for tiles/walls
  // as actors this value will be wrong if they move
  // (which actors are expected to do)
  // int grid_x = 0;
  // int grid_y = 0;

  bool placeholder = true;
};

struct SeenComponent
{
  bool placeholder = true;
};

} // namespace game2d