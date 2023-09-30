#pragma once

#include "entt/entt.hpp"

#include <vector>

namespace game2d {

struct GridComponent
{
  int size = 16; // worldspace size e.g. pixels
  int width = 1;
  int height = 1;
  std::vector<std::vector<entt::entity>> grid;
};

enum class GridDirection : size_t
{
  north,
  south,
  east,
  west,

  // diagonals?
  north_east,
  south_east,
  north_west,
  south_west,
};

} // namespace game2d