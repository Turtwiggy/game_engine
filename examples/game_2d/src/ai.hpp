#pragma once

// other project headers
#include "glm/glm.hpp"

// your project headers
#include "2d_game_object.hpp"

namespace game2d {

namespace enemy_ai {

void
move_along_vector(GameObject2D& obj, glm::vec2 dir, float delta_time_s);

void
enemy_directly_to_player(GameObject2D& obj, GameObject2D& player, float delta_time_s);

void
enemy_arc_angles_to_player(GameObject2D& obj, GameObject2D& player, float delta_time_s);

} // namespace enemy_ai

} // namespace game2d