#pragma once

#include "engine/core/application.hpp"
#include "engine/tools/profiler.hpp"

namespace game_3d {

class ProfilerPanel 
{
public:
    void draw
    (
        const fightingengine::Application& app, 
        const fightingengine::Profiler& profiler, 
        const float delta_time_s
    );

};

} //namespace game_3d