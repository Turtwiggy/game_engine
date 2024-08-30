#pragma once

#include "modules/renderer/components.hpp"

#include <glm/glm.hpp>

namespace game2d {

struct OrthographicCamera
{
  glm::mat4 view = glm::mat4(1.0f);
  glm::mat4 projection = glm::mat4(1.0f);
};

glm::mat4
calculate_ortho_projection(int w, int h, const float dt);

glm::mat4
calculate_ortho_view(const TransformComponent& transform, const float dt);

} // namespace game2d