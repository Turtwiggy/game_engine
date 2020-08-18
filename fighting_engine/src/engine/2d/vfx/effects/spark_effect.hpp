#pragma once

#include <toolkit/render_window.hpp>

#include "random.hpp"
#include "vfx/particle_system.hpp"

//A "one off" effect

namespace vfx {

    struct spark_effect {

        void start(
            render_window& win,
            random_state& rng,
            particle_system& particle_sys,
            vec2f position);

        void update(float delta_time);

    private:

        float time_left = 4.f;

    };

}