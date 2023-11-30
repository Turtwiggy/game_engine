#include "helpers.hpp"

namespace game2d {

glm::quat
vec3_to_quat(const glm::vec3& v)
{
  const glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
  const glm::vec3 forward = glm::vec3(0.0f, 0.0f, -1.0f);
  const glm::vec3 right = glm::vec3(1.0f, 0.0f, 0.0f);
  const auto qx = glm::angleAxis(v.x, glm::vec3(1.0f, 0.0f, 0.0f));
  const auto qy = glm::angleAxis(v.y, glm::vec3(0.0f, 1.0f, 0.0f));
  const auto qz = glm::angleAxis(v.z, glm::vec3(0.0f, 0.0f, 1.0f));

  const float pitch = -v.x;
  const float yaw = -v.y;
  const float roll = -v.z;
  return glm::quat(glm::vec3(pitch, yaw, roll));
}

} // namespace game2d