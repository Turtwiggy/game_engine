#pragma once

#include "game/components/app.hpp"
#include "modules/events/components.hpp"

#include <vector>

namespace game2d {

void
simulate(GameEditor& editor, Game& game, const std::vector<InputEvent>& inputs, uint64_t milliseconds_dt, uint32_t& dungeon_seed);

} // namespace game2d