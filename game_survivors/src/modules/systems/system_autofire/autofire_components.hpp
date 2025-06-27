#pragma once

#include <glm/fwd.hpp>

namespace game2d {

struct AutofireComponent
{
  glm::vec2 draw_cursor_position{ 0, 0 };
  entt::entity target = entt::null;

  float target_aquisition_cooldown_max = 1.0f;
  float target_aquisition_cooldown_cur = 0.0f;

  glm::vec2 dir{ 0, 0 };
};

} // namespace game2d