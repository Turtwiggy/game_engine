//#pragma once
//
//#include <entt/entt.hpp>
//
//#include "vec/vec.hpp"
//#include "sprite_renderer.hpp"
//#include "random.hpp"
//
//namespace vfx {
//
//    enum particle_type
//    {
//        SNOW,
//        SPARK
//    };
//
//    struct particle
//    {
//        render_descriptor desc;
//        sprite_handle sprite;
//        particle_type type;
//
//        float time_total;         //seconds
//        float time_left;          //seconds
//    };
//
//    struct particle_system
//    {
//    public:
//        void emit(particle& p);
//        void render(sprite_renderer& renderer);
//
//        entt::registry& get_particle_registry() { return registry; }
//
//    private:
//        entt::registry registry;
//    };
//
//}
