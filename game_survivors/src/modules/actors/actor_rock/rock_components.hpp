#pragma once

#include "engine/map/components.hpp"
#include <entt/fwd.hpp>

namespace game2d {

struct RockComponent
{
  bool placeholder = true;
};

struct RequestGenerateRocks
{
  bool placeholder = true;
};

struct DebugContoursComponent
{
  std::vector<Edge> edges;
};

} // namespace game2d