//#include "particle_system.hpp"
//
// namespace vfx {
//
//    void particle_system::emit(particle& p)
//    {
//        auto base = registry.create();
//
//        registry.assign<particle>(base, p);
//    }
//
//    void particle_system::render(sprite_renderer& renderer)
//    {
//        auto view = registry.view<particle>();
//
//        for (auto ent : view)
//        {
//            auto& p = view.get<particle>(ent);
//
//            renderer.add(p.sprite, p.desc);
//        }
//    }
//
//}
