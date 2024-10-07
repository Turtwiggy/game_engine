#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace game2d {

bool
vec_ivec2_contains_ivec2(const std::vector<glm::ivec2>& v, const glm::ivec2& t);

} // namespace game2d