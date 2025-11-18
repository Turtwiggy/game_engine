#pragma once

#include "modules/events/event_bump/bump_event_components.hpp"

#include <entt/fwd.hpp>

namespace game2d {

void
handle_bump_event__revive(entt::registry& r, const BumpEvent& evt);

} // namespace game2d