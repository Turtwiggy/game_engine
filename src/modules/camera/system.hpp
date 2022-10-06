#pragma once

#include "game/components/app.hpp"

namespace game2d {

void
init_camera_system(const GameEditor& editor, Game& game);

void
update_camera_system(const GameEditor& editor, Game& game, float dt);

} // namespace game2d
