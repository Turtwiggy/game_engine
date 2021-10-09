#pragma once

namespace game2d {

struct ProfilerStats
{
  float physics_elapsed_ms = 0.0f;
  float input_elapsed_ms = 0.0f;
  float game_tick_elapsed_ms = 0.0f;
  float render_elapsed_ms = 0.0f;
};

} // namespace game2d