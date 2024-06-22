#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>

#include <tuple>
#include <vector>

namespace game2d {

void
generate_intersections(entt::registry& r,
                       const glm::ivec2& light_pos,
                       std::vector<std::tuple<float, float, float>>& intersections);

} // namespace game2d