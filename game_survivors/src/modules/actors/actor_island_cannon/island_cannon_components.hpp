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

  const float time_to_repair = 5;
  float time_to_repair_left = time_to_repair;
};

} // namespace game2d