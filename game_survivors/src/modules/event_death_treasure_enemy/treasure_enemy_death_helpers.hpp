#pragma once

#include "modules/event_death/components.hpp"
#include <entt/entt.hpp>

namespace game2d {

void
handle_death_event__treasure_enemy(entt::registry& r, const DeathEvent& evt);

} // namespace game2d