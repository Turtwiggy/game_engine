//#include "spark_effect.hpp"
//
//namespace vfx {
//
//    void spark_effect::start(
//        render_window& win,
//        random_state& rng,
//        particle_system& particle_sys,
//        vec2f position = { 0.f, 0.f })
//    {
//        auto win_size = win.get_window_size();
//
//        //particle_sys.emit(
//        //    rng,                //random
//        //    position,           //pos
//        //    { -50.0f, -50.f },  //velocity
//        //    { 140 / 255.f, 29 / 255.f, 7 / 255.f, 0.8f },    //start colour
//        //    { 1.f, 0.f, 0.f, 0.f }                          //end colour
//        //);
//    }
//
//    void spark_effect::update(float delta_time)
//    {
//        time_left -= delta_time;
//
//        printf("updating spark effect");
//
//    }
//
//}
