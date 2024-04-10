#pragma once

#include <entt/entt.hpp>

namespace game2d {

// Also attached is a transform component
struct SpaceshipComponent
{
  std::vector<entt::entity> points;
  std::vector<std::pair<int, int>> edges; // index to points
};

struct SpaceshipPointComponent
{
  bool placeholder = true;
};

} // namespace game2d