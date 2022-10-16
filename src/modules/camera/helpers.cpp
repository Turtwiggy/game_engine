// header
#include "helpers.hpp"

#include "modules/camera/components.hpp"
#include "modules/renderer/components.hpp"

#include <glm/gtx/quaternion.hpp>

namespace game2d {

// glm::quat
// get_orientation(const TransformComponent& transform, const CameraComponent& camera)
// {
//   return glm::quat(glm::vec3(-camera.pitch, -camera.yaw, 0.0f));
// };

// glm::vec3
// get_up_dir(const TransformComponent& t, const CameraComponent& c)
// {
//   return glm::rotate(get_orientation(t, c), glm::vec3(0.0f, 1.0f, 0.0f));
// };

// glm::vec3
// get_right_dir(const TransformComponent& t, const CameraComponent& c)
// {
//   return glm::rotate(get_orientation(t, c), glm::vec3(1.0f, 0.0f, 0.0f));
// };

// glm::vec3
// get_forward_dir(const TransformComponent& t, const CameraComponent& c)
// {
//   return glm::rotate(get_orientation(t, c), glm::vec3(0.0f, 0.0f, -1.0f));
// };

glm::mat4
calculate_projection(int w, int h)
{
  // perspective
  // float near_clip = 0.001f;
  // float far_clip = 100.0f;
  // float aspect_ratio = static_cast<float>(w) / static_cast<float>(h);
  // float fov = 60.0f;
  // return glm::perspective(glm::radians(fov), aspect_ratio, near_clip, far_clip);

  // ortho
  const float left = 0.0f;
  const float right = static_cast<float>(w);
  const float bottom = static_cast<float>(h);
  const float top = 0.0f;
  return glm::ortho(left, right, bottom, top, -1.0f, 1.0f);
};

glm::mat4
calculate_view(const TransformComponent& transform, const CameraComponent& camera)
{
  // perspective
  // const auto position = glm::vec3(transform.position.x, transform.position.y, transform.position.z);
  // const glm::quat orientation = get_orientation(transform, camera);
  // glm::mat4 view = glm::translate(glm::mat4(1.0f), position) * glm::toMat4(orientation);
  // view = glm::inverse(view);

  // ortho
  const auto identity = glm::mat4(1.0f);
  const auto position = glm::vec3(transform.position.x, transform.position.y, transform.position.z);
  glm::mat4 view =
    glm::translate(identity, position) * glm::rotate(identity, glm::radians(camera.rotation), glm::vec3(0, 0, 1));
  return glm::inverse(view);
};

} // namespace game2d