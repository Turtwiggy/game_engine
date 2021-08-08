#pragma once

#include "engine/tools/profiler.hpp"

namespace fightingengine {

namespace profiler_panel {

void
draw_timers(const Profiler& profiler, const float delta_time_s);

}

} // namespace fightingengine