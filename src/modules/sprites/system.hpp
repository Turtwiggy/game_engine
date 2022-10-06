#pragma once

#include "game/components/app.hpp"

namespace game2d {

void
init_sprite_system(GameEditor& editor);

void
update_sprite_system(const GameEditor& editor, Game& game, float dt);

} // namespace game2d