#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace game2d {

void
set_position(entt::registry& r, const entt::entity& e, const glm::vec3& position);

} // namespace game2d