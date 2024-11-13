#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace game2d {

struct TilesComponent
{
  std::vector<glm::ivec2> tiles;
};

} // namespace game2d