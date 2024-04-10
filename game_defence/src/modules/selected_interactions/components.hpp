#pragma once

#include <entt/entt.hpp>

#include <vector>

namespace game2d {

struct MapComponent
{
  int xmax = 10;
  int ymax = 10;
  int tilesize = 64;

  std::vector<std::vector<entt::entity>> map;
};

} // namespace game2d