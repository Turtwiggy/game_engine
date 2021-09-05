#pragma once

// other project headers
#include <glm/glm.hpp>

// engine headers
#include "engine/maths_core.hpp"

// game headers
#include "2d_game_state.hpp"

namespace game2d {

namespace enemy_spawner {

void
next_wave(int& enemies_to_spawn_this_wave, int& enemies_to_spawn_this_wave_left, int& wave);

// spawn a random enemy every X seconds
void
update(MutableGameState& state, engine::RandomState& rnd, const glm::ivec2 screen_wh, const float delta_time_s);

} // namespace enemy_spawner

} // namespace game2d