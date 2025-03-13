#pragma once

#include "modules/event_shoot/event_shoot_components.hpp"

#include <entt/fwd.hpp>

namespace game2d {

void
handle_shoot_event__trait_electromancy(entt::registry& r, const ShootEvent& evt);

} // namespace game2d