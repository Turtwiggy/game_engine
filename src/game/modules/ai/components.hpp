#pragma once

#include <entt/entt.hpp>
#include <nlohmann/json.hpp>

//
// AI / pathfinding
//

namespace game2d {

const int k_milliseconds_between_ai_updates = 1000;

struct AiBrainComponent
{
  int milliseconds_between_ai_updates_left = 1000;

  // NLOHMANN_DEFINE_TYPE_INTRUSIVE(AiBrainComponent);
};

struct PathfindableComponent
{
  int cost = 0;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(PathfindableComponent, cost);
};

} // namespace game2d