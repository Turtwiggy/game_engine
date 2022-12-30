#pragma once

#include "components/app.hpp"

namespace game2d {

// take damage on collision
void
check_if_damageable_received_collision(Game& game);

void
check_if_enemy_collided_with_cursor(Game& game);

void
check_if_player_collided_with_pickup(Game& game);

void
check_if_player_collided_with_exit(Game& game);

} // namespace game2d