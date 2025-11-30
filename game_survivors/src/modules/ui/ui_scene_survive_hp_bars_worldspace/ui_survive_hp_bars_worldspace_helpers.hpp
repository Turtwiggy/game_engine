#pragma once

#include "modules/events/event_damage/event_damage_components.hpp"
#include <entt/fwd.hpp>

namespace game2d {

void
handle_damage_event__worldspace_hp_bars(entt::registry& r, const TookDamageEvent& evt);

} // namespace game2d