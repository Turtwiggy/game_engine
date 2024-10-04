#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace game2d {

void
move_action_common(entt::registry& r, const entt::entity e, const glm::vec2& dst_wp);

} // namespace game2d