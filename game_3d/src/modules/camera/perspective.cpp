#include "perspective.hpp"

#include <glm/gtx/quaternion.hpp>

namespace game2d {

glm::mat4
calculate_perspective_projection(int w, int h)
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
  glm::mat4 view(1.0f);
  const auto orientation = get_orientation(camera);
  view = glm::translate(glm::mat4(1.0), transform.position) * glm::toMat4(orientation);
  view = glm::inverse(view);
  return view;

  return glm::lookAt(transform.position, transform.position + get_forward_dir(camera), get_up_dir(camera));
};

glm::quat
get_orientation(const PerspectiveCamera& camera)
{
  return glm::quat(glm::vec3(-camera.pitch, -camera.yaw, 0.0f));
};

glm::vec3
get_up_dir(const PerspectiveCamera& c)
{
  return glm::rotate(get_orientation(c), glm::vec3(0.0f, 1.0f, 0.0f));
};

glm::vec3
get_right_dir(const PerspectiveCamera& c)
{
  return glm::rotate(get_orientation(c), glm::vec3(1.0f, 0.0f, 0.0f));
};

glm::vec3
get_forward_dir(const PerspectiveCamera& c)
{
  return glm::rotate(get_orientation(c), glm::vec3(0.0f, 0.0f, -1.0f));
};

} // namespace