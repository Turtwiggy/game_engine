#pragma once

#include <entt/fwd.hpp>

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
};

} // namespace game2d