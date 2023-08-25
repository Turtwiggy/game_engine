#pragma once

#include <entt/entt.hpp>

#include <glm/glm.hpp>

namespace game2d {

struct BowComponent
{
  entt::entity parent = entt::null;

  float lerp_speed = 25.0f;
  float lerp_amount = 0.0f; // 0-1
  glm::ivec2 target_position{ 0, 0 };
};

}; // namespace game2d