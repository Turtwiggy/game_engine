#include "helpers.hpp"

namespace game2d {

glm::quat
vec3_to_quat(const glm::vec3& v)
{
  const auto qx = glm::angleAxis(v.x, glm::vec3(1.0f, 0.0f, 0.0f));
  const auto qy = glm::angleAxis(v.y, glm::vec3(0.0f, 1.0f, 0.0f));
  const auto qz = glm::angleAxis(v.z, glm::vec3(0.0f, 0.0f, 1.0f));
  return qx * qy * qz;
}

} // namespace game2d