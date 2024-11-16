#pragma once

#include "modules/resolve_collisions/resolve_collisions_helpers.hpp"
#include <entt/entt.hpp>

namespace game2d {

void
handle_unit_enter_blackhole(entt::registry& r, const GridPositionChangedEvent& evt);

} // namespace game2d