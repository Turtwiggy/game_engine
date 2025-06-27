#pragma once

#include "engine/map/components.hpp"

namespace game2d {

bool
crossing_number_algorithm__point_is_inside(const glm::vec2& point, const std::vector<Edge>& polygon);
} // namespace game2d