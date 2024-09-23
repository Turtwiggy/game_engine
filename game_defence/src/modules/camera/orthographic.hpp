#pragma once

#include "engine/renderer/transform.hpp"

#include <glm/glm.hpp>

namespace game2d {

struct OrthographicCamera
{
  glm::mat4 view = glm::mat4(1.0f);
  glm::mat4 projection = glm::mat4(1.0f);
  glm::mat4 projection_zoomed = glm::mat4(1.0f);

  float zoom_linear = 0.0f;    // [-1, 1]
  float zoom_nonlinear = 1.0f; // zoom to better UX values
};

glm::mat4
calculate_ortho_projection(int w, int h, float zoom);

glm::mat4
calculate_ortho_view(const TransformComponent& transform, const float dt);

} // namespace game2d