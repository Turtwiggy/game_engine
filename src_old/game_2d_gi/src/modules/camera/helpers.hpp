#pragma once

#include "entt/entt.hpp"

#include <glm/glm.hpp>

namespace game2d {

[[nodiscard]] glm::ivec2
mouse_position_in_worldspace(entt::registry& r);

} // namespace game2d