#pragma once

#include "components.hpp"

#include <entt/entt.hpp>

namespace game2d {

void
update_screenshake_system(entt::registry& r, const float timer, const float dt);

} // namespace game2d