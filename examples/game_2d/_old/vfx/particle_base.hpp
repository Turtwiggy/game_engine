//#pragma once
//
//#include "quad_renderer.hpp"
//#include "random.hpp"
//#include "particle_system.hpp"
//#include "tilemap.hpp"
//
// namespace vfx {
//
//    struct particle_base
//    {
//        void start() { paused = false; };
//        void stop() { paused = true; }
//
//        void update(float delta_time,
//            render_window& win,
//            random_state& rng,
//            particle_system& particle_sys)
//        {
//            if (paused)
//                return;
//
//            spawn_particle_check(delta_time, particle_sys, win, rng);
//            update_all_particles(delta_time, particle_sys);
//        };
//
//        // if a particle should spawn
//        virtual void spawn_particle_check(
//            float delta_time,
//            particle_system& particle_sys,
//            render_window& win,
//            random_state& rng) = 0;
//
//        // update all particles movement
//        virtual void update_all_particles(
//            float delta_time,
//            particle_system& particle_sys) = 0;
//
//    protected:
//
//        bool paused = true;
//    };
//
//}
