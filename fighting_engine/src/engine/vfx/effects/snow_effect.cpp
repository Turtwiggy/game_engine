//#include "snow_effect.hpp"
//
//#include "sprite_renderer.hpp"
//#include <imgui/imgui.h>
//
//
//namespace vfx {
//
//    void snow_effect::snow_effect_editor()
//    {
//        ImGui::Begin("Snow effect editor");
//
//        float list[2] = { velocity.x(), velocity.y() };
//        ImGui::SliderFloat2("Velocity", list, -100., 100.);
//        velocity.x() = list[0];
//        velocity.y() = list[1];
//
//        float start_col[4] = { colour_start.x(), colour_start.y(), colour_start.z(), colour_start.w() };
//        ImGui::ColorEdit4("StartCol", start_col);
//        colour_start = vec4f{ start_col[0], start_col[1], start_col[2], start_col[3] };
//
//        float end_col[4] = { colour_end.x(), colour_end.y(), colour_end.z(), colour_end.w() };
//        ImGui::ColorEdit4("EndCol", end_col);
//        colour_end = vec4f{ end_col[0], end_col[1], end_col[2], end_col[3] };
//
//        ImGui::End();
//    }
//
//    void snow_effect::spawn_particle_check(float delta_time,
//        particle_system& particle_sys,
//        render_window& win,
//        random_state& rng)
//    {
//        time_left_before_new_particle -= delta_time;
//
//        if (time_left_before_new_particle > 0.f)
//            return;
//
//        time_left_before_new_particle = time_between_particles;
//
//        spawn_particle(win, rng, particle_sys);
//    }
//
//    void snow_effect::spawn_particle(
//        render_window& win,
//        random_state& rng,
//        particle_system& particle_sys)
//    {
//        auto win_size = win.get_window_size();
//
//        float max_x = rand_det_s(rng.rng, 0.0, 1.0) * win_size.x();
//        float max_y = rand_det_s(rng.rng, 0.0, 1.0) * win_size.y();
//
//        particle p;
//        p.sprite = get_sprite_handle_of(rng, sprite);
//        p.type = type;
//        p.time_total = particle_time_total;
//        p.time_left = particle_time_total;
//
//        render_descriptor desc;
//        desc.pos = { max_x, max_y };
//        p.desc = desc;
//
//        particle_sys.emit(p);
//    }
//
//    void snow_effect::update_all_particles(float delta_time, particle_system& particle_sys)
//    {
//        snow_effect_editor();
//
//        entt::registry& reg = particle_sys.get_particle_registry();
//
//        auto view = reg.view<particle>();
//
//        for (auto ent : view)
//        {
//            auto& p = view.get<particle>(ent);
//
//            if (p.type != type)
//                return;
//
//            //reduce time_left
//            p.time_left -= delta_time;
//            if (p.time_left < 0.0f)
//            {
//                reg.destroy(ent);
//                continue;
//            }
//
//            //goes from 1 to 0
//            float life = p.time_left / p.time_total;
//
//            render_descriptor desc = p.desc;
//
//            //Update position
//            desc.pos += velocity * delta_time;
//               
//            //Update angle
//            desc.angle += angle_every_frame * delta_time;
//
//            //Update colour
//            vec4f lerped_colour = mix(colour_end, colour_start, life);
//            p.sprite.base_colour = lerped_colour; //works
//            //p.desc.colour = lerped_colour;      //does not work
//
//            //Update size
//            vec2f lerped_size = mix(size_end, size_begin, life);
//            desc.scale = lerped_size;
//
//            p.desc = desc;
//        }
//    }
//}
//
