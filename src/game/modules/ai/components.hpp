#pragma once

#include <entt/entt.hpp>
#include <nlohmann/json.hpp>

//
// AI / pathfinding
//

namespace game2d {

struct vec2i
{
  int x = 0;
  int y = 0;

  // spaceship operator
  auto operator<=>(const vec2i&) const = default;
};

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

struct FollowPathComponent
{
  std::vector<vec2i> calculated_path;

  int k_milliseconds_between_path_updates_left = 1000;
};

} // namespace game2d