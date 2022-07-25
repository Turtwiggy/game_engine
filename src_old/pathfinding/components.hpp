#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace game2d {

struct DestinationComponent
{
  // state
  glm::ivec2 xy = glm::ivec2(0);

  // ui
  entt::entity destination_line = entt::null;
};

} // namespace game2d