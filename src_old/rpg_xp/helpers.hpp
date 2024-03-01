#pragma once

namespace game2d {

// some XP to Level algorithm
// e.g. 200xp => lv 2
// e.g. 350xp => lv 3
// e.g. 500xp => lv 4
// e.g. 650xp => lv 5 .. etc
int
convert_xp_to_level(const int xp);

} // namespace game2d