#pragma once

#include "entt/entt.hpp"

#include <glm/glm.hpp>

namespace game2d {

[[nodiscard]] glm::vec2
mouse_position_in_worldspace(entt::registry& r);

[[nodiscard]] glm::ivec2
position_in_worldspace(entt::registry& r, const glm::ivec2& pos);

} // namespace game2d