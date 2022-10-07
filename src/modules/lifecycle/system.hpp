#pragma once

#include "game/components/app.hpp"

namespace game2d {

void
update_lifecycle_system(GameEditor& editor, Game& game, uint64_t milliseconds_dt);

} // namespace game2d