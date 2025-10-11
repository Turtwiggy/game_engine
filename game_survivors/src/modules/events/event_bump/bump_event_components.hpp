#pragma once

#include <entt/fwd.hpp>

namespace game2d {

struct BumpEvent
{
  entt::entity from;
  entt::entity to;
  glm::ivec2 from_gp;
  glm::ivec2 to_gp;
};

} // namespace game2d