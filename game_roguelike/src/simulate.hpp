#pragma once

#include "components/app.hpp"
#include "events/components.hpp"

#include <vector>

namespace game2d {

void
simulate(GameEditor& editor, Game& game, const std::vector<InputEvent>& inputs, uint64_t milliseconds_dt);

} // namespace game2d