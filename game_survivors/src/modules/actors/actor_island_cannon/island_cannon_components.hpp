#pragma once

#include <entt/fwd.hpp>

#include "engine/maths/grid.hpp"

namespace game2d {

enum class IslandCannonState
{
  BROKEN,
  WORKING,
};

struct IslandCannonComponent
{
  IslandCannonState state = IslandCannonState::BROKEN;

  int hits_to_repair = 3;
  int hits_to_repair_left = hits_to_repair;

  bool initialized_edges = false;
  std::vector<std::pair<engine::grid::GridDirection, glm::ivec2>> water_edges;
  std::vector<std::pair<engine::grid::GridDirection, glm::ivec2>> land_edges;
};

} // namespace game2d