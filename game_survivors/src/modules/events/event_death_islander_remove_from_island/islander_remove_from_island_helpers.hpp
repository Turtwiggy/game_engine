#pragma once

#include "modules/events/event_death/components.hpp"

#include <entt/fwd.hpp>

namespace game2d {

void
handle_death_event__islander_remove_from_island(entt::registry& r, const DeathEvent& evt);

} // namespace game2d