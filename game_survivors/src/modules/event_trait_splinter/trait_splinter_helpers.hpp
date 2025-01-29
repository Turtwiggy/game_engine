#pragma once

#include "modules/event_permadeath/event_permadeath_components.hpp"

#include <entt/entt.hpp>

namespace game2d {

void
handle_death_event__trait_splinter(entt::registry& r, const DeathEvent& evt);

} // namespace game2d