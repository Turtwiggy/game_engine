#pragma once

#include "engine/map/components.hpp"
#include <entt/fwd.hpp>

namespace game2d {

struct BoundingBoxComponent
{
  glm::vec2 tl{ FLT_MAX, FLT_MAX };
  glm::vec2 br{ -FLT_MAX, -FLT_MAX };
};

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
  std::vector<Edge> sorted_edges;
};

} // namespace game2d