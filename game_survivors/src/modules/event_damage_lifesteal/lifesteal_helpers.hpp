#pragma once

#include "modules/event_damage/event_damage_components.hpp"
#include <entt/entt.hpp>

namespace game2d {

void
handle_damage_event_lifesteal(entt::registry& r, const DamageEvent& evt);

} // namespace game2d