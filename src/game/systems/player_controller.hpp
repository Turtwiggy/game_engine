#pragma once

#include "modules/events/components.hpp"

#include <entt/entt.hpp>

#include <vector>

namespace game2d {

void
update_player_controller_system(entt::registry& r, const std::vector<InputEvent>& inputs);

} // namespace game2d