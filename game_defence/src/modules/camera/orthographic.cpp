#include "orthographic.hpp"

namespace game2d {

glm::mat4
calculate_ortho_projection(int w, int h)
{
  // ToDo: implement camera zoom
  const float scale = 1.5f;

  const float left = 0.0f;
  const float right = static_cast<float>(w);
  const float top = static_cast<float>(h);
  const float bottom = 0.0f;

  return glm::ortho(left, right, bottom, top, -1.0f, 1.0f);
}

glm::mat4
calculate_ortho_view(const TransformComponent& transform)
{
  const auto identity = glm::mat4(1.0f);
  const auto position = glm::vec3(transform.position.x, transform.position.y, transform.position.z);
  auto view = glm::translate(identity, position) * glm::rotate(identity, transform.rotation_radians.z, glm::vec3(0, 0, 1));
  return glm::inverse(view);
};

}