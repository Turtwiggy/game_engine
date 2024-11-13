#pragma once

#include "modules/event_damage/event_damage_helpers.hpp"
#include <entt/entt.hpp>

namespace game2d {

void
handle_damage_event_apply_bleed(entt::registry& r, const DamageEvent& evt);

} // namespace game2d