#pragma once

#include <glm/fwd.hpp>

namespace game2d {

struct AutofireComponent
{
  glm::vec2 draw_cursor_position{ 0, 0 };
  entt::entity target = entt::null;

  glm::vec2 dir{ 0, 0 };
};

} // namespace game2d