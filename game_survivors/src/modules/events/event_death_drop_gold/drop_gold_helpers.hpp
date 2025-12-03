#pragma once

#include "modules/events/event_death/components.hpp"

#include <entt/fwd.hpp>

namespace game2d {

void
handle_death_event__drop_gold(entt::registry& r, const DeathEvent& evt);

} // namespace game2d