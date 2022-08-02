#pragma once

// other lib headers
#include <entt/entt.hpp>

namespace game2d {

void
init_input_system(entt::registry& registry);

void
update_input_system(entt::registry& registry);

} // namespace game2d