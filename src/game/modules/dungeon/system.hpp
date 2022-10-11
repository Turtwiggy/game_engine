#pragma once

#include "game/components/app.hpp"

#include "components.hpp"

namespace game2d {

// If the room generation algorithm hits "step" rooms
// the algorithm is stopped. This can be useful for debugging
// by pausing it at "step" 1, 2, 3, 4 and seeing what the
// rooms look like at that "step".
void
generate_dungeon(GameEditor& editor, Game& game, const Dungeon& d);

void
update_dungeon_system(GameEditor& editor, Game& game);

} // namespace game2d