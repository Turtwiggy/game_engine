#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

namespace game2d {

glm::quat
vec3_to_quat(const glm::vec3& v);

} // namespace game2d