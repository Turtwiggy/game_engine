#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>

#include <vector>

namespace game2d {

struct LegsComponent
{
  entt::entity body = entt::null;

  std::vector<entt::entity> lines;
  glm::ivec2 hip{ 0, 0 };
  glm::ivec2 feet{ 0, 0 };
};

} // namespace game2d