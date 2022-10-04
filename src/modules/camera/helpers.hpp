#pragma once

#include "modules/camera/components.hpp"
#include "modules/renderer/components.hpp"

#include <glm/glm.hpp>

namespace game2d {

// update if viewport changes size
glm::mat4
calculate_projection(int x, int y);

// update when camera moves
glm::mat4
calculate_view(const TransformComponent& transform, const CameraComponent& camera);

} // namespace game2d