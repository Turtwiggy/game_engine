#pragma once

#include "modules/renderer/components.hpp"

#include <glm/glm.hpp>

namespace game2d {

struct PerspectiveCamera
{
  glm::mat4 view = glm::mat4(1.0f);
  glm::mat4 projection = glm::mat4(1.0f);
  float pitch = 0.0f;
  float yaw = 0.0f;
  float speed = 2.5f;
};

glm::mat4
calculate_perspective_projection(int w, int h);

glm::mat4
calculate_perspective_view(const TransformComponent& t, const PerspectiveCamera& camera);

glm::quat
get_orientation(const PerspectiveCamera& camera);

glm::vec3
get_up_dir(const PerspectiveCamera& c);

glm::vec3
get_right_dir(const PerspectiveCamera& c);

glm::vec3
get_forward_dir(const PerspectiveCamera& c);

} // namespace game2d
