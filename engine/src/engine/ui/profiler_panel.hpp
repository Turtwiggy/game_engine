#pragma once

#include "engine/core/application.hpp"
#include "engine/tools/profiler.hpp"

namespace fightingengine {

class ProfilerPanel 
{
public:

void draw
(
    const Application& app, 
    const Profiler& profiler, 
    const float delta_time_s
);

};

} //namespace fightingengine