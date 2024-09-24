#include "orthographic.hpp"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

namespace game2d {

glm::mat4
calculate_ortho_projection(int w, int h, float zoom)
{
  if (zoom < 0.0f)
    zoom = 0.0f;

  float left = 0.0f;
  float right = static_cast<float>(w);
  float top = static_cast<float>(h);
  float bottom = 0.0f;

  float center_x = right / 2.0f;
  float center_y = top / 2.0f;
  left = center_x - (center_x * zoom);
  right = center_x + (center_x * zoom);
  top = center_y + (center_y * zoom);
  bottom = center_y - (center_y * zoom);

  return glm::ortho(left, right, bottom, top, -1.0f, 1.0f);
};

glm::mat4
calculate_ortho_view(const TransformComponent& transform, const float dt)
{
  const auto identity = glm::mat4(1.0f);
  const auto position = glm::vec3(transform.position.x, transform.position.y, transform.position.z);

  glm::mat4 view = glm::translate(identity, position);
  view *= glm::rotate(identity, transform.rotation_radians.z, glm::vec3(0, 0, 1));

  return glm::inverse(view);
};

}