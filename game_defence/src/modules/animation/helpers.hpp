#pragma once

#include "components.hpp"
#include "modules/physics/components.hpp"
#include "renderer/components.hpp"
#include <glm/glm.hpp>

namespace game2d {

AABB
generate_aabb(const RotatedSquare& square);

bool
point_in_rectangle(const RotatedSquare& square, glm::ivec2 point);

bool
rotated_squares_collide(const RotatedSquare& a, const RotatedSquare& b);

RotatedSquare
transform_to_rotated_square(const TransformComponent& t);

} // namespace game2d