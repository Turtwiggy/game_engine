#pragma once

#include "engine/maths/grid.hpp"

#include <entt/fwd.hpp>

namespace game2d {

struct IslandLineInfo
{
  bool initialized_edges = false;
  std::vector<std::pair<engine::grid::GridDirection, glm::ivec2>> water_edges;
  std::vector<std::pair<engine::grid::GridDirection, glm::ivec2>> land_edges;
};

struct RevivableComponent
{
  int hits_to_revive = 3;
  int hits_to_revive_cur = 0;
};

} // namespace game2d