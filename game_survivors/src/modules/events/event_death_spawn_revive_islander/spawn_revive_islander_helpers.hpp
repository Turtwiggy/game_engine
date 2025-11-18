#pragma once

#include <entt/fwd.hpp>

#include "modules/events/event_death/components.hpp"

namespace game2d {

void
handle_death_event__spawn_revive_islander(entt::registry& r, const DeathEvent& evt);

} // namespace game2d