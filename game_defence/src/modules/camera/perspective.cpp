#include "perspective.hpp"

#include <glm/gtx/quaternion.hpp>

namespace game2d {

glm::mat4
game2d::calculate_perspective_projection(int w, int h)
{
  float near_clip = 0.001f;
  float far_clip = 100.0f;
  float aspect_ratio = static_cast<float>(w) / static_cast<float>(h);
  float fov = 60.0f;
  return glm::perspective(glm::radians(fov), aspect_ratio, near_clip, far_clip);
};

glm::mat4
calculate_perspective_view(const TransformComponent& transform, const PerspectiveCamera& camera)
{
  const auto position = glm::vec3(transform.position.x, transform.position.y, transform.position.z);
  const glm::quat orientation = get_orientation(transform, camera);
  glm::mat4 view = glm::translate(glm::mat4(1.0f), position) * glm::toMat4(orientation);
  return glm::inverse(view);
};

glm::quat
get_orientation(const TransformComponent& transform, const PerspectiveCamera& camera)
{
  return glm::quat(glm::vec3(-camera.pitch, -camera.yaw, 0.0f));
};

glm::vec3
get_up_dir(const TransformComponent& t, const PerspectiveCamera& c)
{
  return glm::rotate(get_orientation(t, c), glm::vec3(0.0f, 1.0f, 0.0f));
};

glm::vec3
get_right_dir(const TransformComponent& t, const PerspectiveCamera& c)
{
  return glm::rotate(get_orientation(t, c), glm::vec3(1.0f, 0.0f, 0.0f));
};

glm::vec3
get_forward_dir(const TransformComponent& t, const PerspectiveCamera& c)
{
  return glm::rotate(get_orientation(t, c), glm::vec3(0.0f, 0.0f, -1.0f));
};

} // namespace game2d