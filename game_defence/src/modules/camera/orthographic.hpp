#pragma once

#include "renderer/components.hpp"

#include <glm/glm.hpp>

namespace game2d {

struct OrthographicCamera
{
  glm::mat4 view = glm::mat4(1.0f);
  glm::mat4 projection = glm::mat4(1.0f);
};

glm::mat4
calculate_ortho_projection(int w, int h);

glm::mat4
calculate_ortho_view(const TransformComponent& transform);

} // namespace game2d