#pragma once

#include "modules/ui_action_bar/ui_action_bar_components.hpp"

#include <entt/entt.hpp>

namespace game2d {

void
handle_end_turn_tutorial(entt::registry& r, const EndTurnEvent& evt);

} // namespace game2d