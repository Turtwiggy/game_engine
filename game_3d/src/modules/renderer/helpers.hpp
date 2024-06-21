#pragma once

#include "components.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

namespace game2d {

glm::quat
vec3_to_quat(const glm::vec3& v);

bool
check_if_viewport_resize(const SINGLE_RendererComponent& ri);

} // namespace game2d