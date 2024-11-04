#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace game2d {

struct TilesComponent
{
  std::vector<glm::ivec2> tiles;
};

enum class RangeType
{
  knife = 0,
  pistol,
  shotgun,
  plunger,

  count,
};

} // namespace game2d