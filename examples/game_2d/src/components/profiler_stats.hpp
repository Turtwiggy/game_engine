#pragma once

namespace game2d {

struct ProfilerStats
{
  float physics_elapsed_ms;
  float input_elapsed_ms;
  float game_tick_elapsed_ms;
  float render_elapsed_ms;
};

} // namespace game2d