#pragma once

#include "event_permadeath_components.hpp"
#include <entt/entt.hpp>

namespace game2d {

void
handle_death_event_set_unit_as_dead(entt::registry& r, const DeathEvent& evt);

} // namespace game2d