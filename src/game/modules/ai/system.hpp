#pragma once

#include "game/components/app.hpp"
#include "game/modules/dungeon/components.hpp"

namespace game2d {

void
update_ai_system(GameEditor& editor, Game& game, const uint64_t& milliseconds_dt);

} // namespace game2d