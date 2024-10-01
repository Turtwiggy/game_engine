#pragma once

#include "modules/event_damage/event_damage_helpers.hpp"

#include <entt/entt.hpp>

namespace game2d {

void
handle_damage_event_for_ui(entt::registry& r, const DamageEvent& evt);

void
update_ui_combat_damage_numbers_system(entt::registry& r, const float dt);

} // namespace game2d