#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace game2d {

void
set_line(entt::registry& r, const entt::entity& e, const glm::ivec2& a, const glm::ivec2& b);

} // game2d