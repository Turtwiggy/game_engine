#pragma once

#include "modules/events/event_shoot/event_shoot_components.hpp"
#include <entt/fwd.hpp>

namespace game2d {

void
handle_shoot_event__audio(entt::registry& r, const ShootEvent& evt);

} // namespace game2d
