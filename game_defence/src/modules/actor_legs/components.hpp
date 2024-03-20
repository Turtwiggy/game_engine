#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>

#include <vector>

namespace game2d {

struct LegsComponent
{
  entt::entity body = entt::null;

  std::vector<entt::entity> lines;

  float timer = 0;

  glm::vec2 r_hip{ 0, 0 };
  glm::vec2 r_knee{ 0, 0 };
  glm::vec2 r_foot{ 0, 0 };
  glm::vec2 r_foot_target{ 0, 0 };

  glm::vec2 l_hip{ 0, 0 };
  glm::vec2 l_knee{ 0, 0 };
  glm::vec2 l_foot{ 0, 0 };
  glm::vec2 l_foot_target{ 0, 0 };
};

} // namespace game2d