#pragma once

#include "game/components/app.hpp"

namespace game2d {

void
init_networking_system(const GameEditor& editor);

void
update_networking_system(GameEditor& editor, Game& game, uint64_t milliseconds_dt);

} // namespace game2d
