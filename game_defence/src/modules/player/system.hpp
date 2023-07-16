#pragma once

#include "events/components.hpp"
#include <entt/entt.hpp>
#include <vector>

namespace game2d {

void
update_player_controller_system(entt::registry& r, const std::vector<InputEvent>& inputs, const uint64_t& milliseconds_dt);

} // namespace game2d