#pragma once

#include <toolkit/render_window.hpp>
#include <vector>
#include <entt/entt.hpp>

#include "random.hpp"
#include "vfx/particle_system.hpp"
#include "vfx/particle_base.hpp"
#include "tilemap.hpp"

namespace vfx {

    struct snow_effect : vfx::particle_base {

    public:

        void editor()
        {
            ImGui::Begin("Effects");

            if (ImGui::Button("Start Snow"))
                start();

            if (ImGui::Button("Pause Snow"))
                stop();

            ImGui::End();
        }

    protected:

        void spawn_particle_check(
            float delta_time,
            particle_system& particle_sys,
            render_window& win,
            random_state& rng) override;

        void spawn_particle(
            render_window& win,
            random_state& rng,
            particle_system& particle_sys);

        void update_all_particles(
            float delta_time,
            particle_system& particle_sys);

        void snow_effect_editor();

        //Visual information
        particle_type type = particle_type::SNOW;
        tiles::type sprite = tiles::EFFECT_10;

        //Particle information
        vec2f velocity = { 55.f, 40.f };
        vec2f size_begin{ 1.f, 1.f };
        vec2f size_end{ 0.25f, 0.25f };
        float particle_time_total = 4.f;
        vec4f colour_end = vec4f{ 0.f, 0.f, 1.f, 0.f };
        vec4f colour_start = vec4f{ 1.f, 0.f, 0.f, 1.f };

        //Note: maximum particles on the screen is:
        //particles_per_second * particle_time_total

        //Effect information
        float particles_per_second = 20.0f; // EDIT THIS
        float time_between_particles = 1. / particles_per_second;
        float time_left_before_new_particle = time_between_particles;

        float seconds_to_rotate_360_degrees = 1.f; //EDIT THIS
        float angle_every_frame = M_PIf / seconds_to_rotate_360_degrees;
    };
}
