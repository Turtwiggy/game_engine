#pragma once

#include "modules/renderer/components.hpp"

#include <glm/glm.hpp>

namespace game2d {

struct OrthographicCamera
{
  glm::mat4 view = glm::mat4(1.0f);
  glm::mat4 projection = glm::mat4(1.0f);

  // stored here instead of TransformComponent
  glm::ivec2 base_position{ 0, 0 };
  glm::ivec2 offset_with_mouse{ 0, 0 };
};

glm::mat4
calculate_ortho_projection(int w, int h);

glm::mat4
calculate_ortho_view(const TransformComponent& transform);

} // namespace game2d