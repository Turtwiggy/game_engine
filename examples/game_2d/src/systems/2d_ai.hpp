#pragma once

// standard libs
#include <vector>

// your project libs
#include "game/2d_game_object.hpp"

namespace game2d {

void
ai_chase_player(GameObject2D& player, const float delta_time, std::vector<GameObject2D>& enemies);

} // namespace game2d