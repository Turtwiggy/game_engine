#pragma once

#include "game/components/app.hpp"
#include "modules/events/components.hpp"

#include <vector>

namespace game2d {

void
update_player_controller_system(GameEditor& editor,
                                Game& game,
                                const std::vector<InputEvent>& inputs,
                                const uint64_t& milliseconds_dt);

} // namespace game2d