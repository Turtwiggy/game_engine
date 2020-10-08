#define _CRT_SECURE_NO_WARNINGS
#define IMGUI_IMPL_OPENGL_LOADER_GLEW

#include "engine/core/application.hpp"
//#include "engine/resources/model_manager.hpp"
#include "engine/3d/game_object_3d.hpp"
#include "engine/3d/camera.hpp"
#include "engine/3d/renderer/renderer_ray_traced.hpp"
#include "engine/renderer/render_command.hpp"
#include "engine/renderer/shader.hpp"
#include "engine/geometry/triangle.hpp"

//game_3d specific code
#include "panels/scene_hierarchy_panel.hpp"
#include "game_state.hpp"

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <SDL2/SDL.h>
#undef main
//^thanks SDL2

#include <memory>
#include <vector>

using namespace fightingengine;
using namespace game_3d;

int main(int argc, char** argv)
{
    int width = 1366;
    int height = 768;
    Application app("Fighting Game!", width, height);
    app.set_fps_limit(144.0);

    //Camera
    // auto viewport_height = 2.0;
    // auto viewport_width = viewport_height * aspect_ratio;

    //Create world (for now, just spheres)
    std::vector<Sphere> world = create_world();

    //Camera
    Camera camera = Camera (
        glm::vec3(0.0f, 0.0f, 1.0f), 
        glm::vec3(0.0f, 1.0f, 0.0f) );
    
    //ModelManager model_manager;
    GameState state;
    //state.init(model_manager);
    RandomState rnd;

    RendererRayTraced renderer;
    renderer.init(width, height);

    float timer = 0.0f;
    while (app.is_running())
    {
        app.frame_begin();
        app.poll();
        float delta_time_s = app.get_delta_time();
        timer += delta_time_s;

        //Shutdown app
        if (app.get_input().get_key_down(SDL_KeyCode::SDLK_END))
            app.shutdown();

        //Toggle mouse capture
        if (app.get_input().get_key_down(SDL_KeyCode::SDLK_m))
            app.get_window().ToggleMouseCaptured();

        //Fullscreen
        if (app.get_input().get_key_down(SDL_KeyCode::SDLK_f))
        {
            app.get_window().ToggleFullscreen();
            int width, height;
            app.get_window().GetSize(width, height);
            std::cout << "screen size toggled, w: " << width << " h: " << height << std::endl;
            renderer.resize(camera, width, height);
        }

        //TODO EVENT: resized window
        // if (app.get_event().window_resized)
        //    renderer.resize(width, height);

        // ~~ Camera ~~
        if(app.get_input().get_key_held(SDL_Scancode::SDL_SCANCODE_W))
            camera.update(delta_time_s, CameraMovement::FORWARD);
        else if(app.get_input().get_key_held(SDL_Scancode::SDL_SCANCODE_S))
            camera.update(delta_time_s, CameraMovement::BACKWARD);
        if (app.get_input().get_key_held(SDL_Scancode::SDL_SCANCODE_A))
            camera.update(delta_time_s, CameraMovement::LEFT);
        else if (app.get_input().get_key_held(SDL_Scancode::SDL_SCANCODE_D))
            camera.update(delta_time_s, CameraMovement::RIGHT);
        if (app.get_input().get_key_held(SDL_Scancode::SDL_SCANCODE_SPACE))
            camera.update(delta_time_s, CameraMovement::UP);
        else if (app.get_input().get_key_held(SDL_Scancode::SDL_SCANCODE_LSHIFT))
            camera.update(delta_time_s, CameraMovement::DOWN);

        // ~~ Mouse ~~
        if(app.get_window().IsInputGrabbed())
        {
            int x, y;
            SDL_GetRelativeMouseState(&x, &y);
            camera.process_mouse_movement(x, y, true);
        }

        // ~~ rendering ~~
        state.render(renderer, camera, app.get_window(), timer);

        // ~~ GUI ~~
        app.gui_begin();

        ImGui::Begin("Hello Window");
        ImGui::Text("Hello World");
        ImGui::End();

        ImGui::Begin("Info");
        ImGui::Text("FPS: %f", app.get_average_fps());
        ImGui::End();

        app.gui_end();

        app.frame_end(delta_time_s);
    }

    return 0;
}








// ~~~~~~~~~ Profiling ~~~~~~~~~~~~~~

//// input
//// -----
//profiler.Begin(Profiler::Stage::SdlInput);
//profiler.End(Profiler::Stage::SdlInput);
//
//// Game State Tick
//// ---------------
//{
//    profiler.Begin(Profiler::Stage::GameTick);
//
//    seconds_since_last_game_tick += delta_time_in_seconds;
//
//    //while (seconds_since_last_game_tick >= SECONDS_PER_FIXED_TICK)
//    //{
//    //    //Fixed update
//    //    fixed_tick(SECONDS_PER_FIXED_TICK);
//    //    seconds_since_last_game_tick -= SECONDS_PER_FIXED_TICK;
//    //}
//
//    profiler.End(Profiler::Stage::GameTick);
//}

//ImGui::Begin("Profiler");
//
//ImGui::Columns(1);
//ImGui::Text("FPS: %f", game.fps_buffer.average());
//
//ImGui::Columns(1);
//
//float draw_time = profiler.GetTime(Profiler::Stage::SdlInput);
//ImGui::Text("%s %f ms", profiler.stageNames[(uint8_t)Profiler::Stage::SdlInput].data(), (draw_time));
//
//draw_time = profiler.GetTime(Profiler::Stage::GameTick);
//ImGui::Text("%s % fms", profiler.stageNames[(uint8_t)Profiler::Stage::GameTick].data(), (draw_time));
//
//draw_time = profiler.GetTime(Profiler::Stage::NewFrame);
//ImGui::Text("%s %f ms", profiler.stageNames[(uint8_t)Profiler::Stage::NewFrame].data(), (draw_time));
//
//draw_time = profiler.GetTime(Profiler::Stage::MainDraw);
//ImGui::Text("%s %f ms", profiler.stageNames[(uint8_t)Profiler::Stage::MainDraw].data(), (draw_time));
//
//draw_time = profiler.GetTime(Profiler::Stage::GuiLoop);
//ImGui::Text("%s % fms", profiler.stageNames[(uint8_t)Profiler::Stage::GuiLoop].data(), (draw_time));
//
//draw_time = profiler.GetTime(Profiler::Stage::EndFrame);
//ImGui::Text("%s % fms", profiler.stageNames[(uint8_t)Profiler::Stage::EndFrame].data(), (draw_time));
//
//draw_time = profiler.GetTime(Profiler::Stage::Sleep);
//ImGui::Text("%s % fms", profiler.stageNames[(uint8_t)Profiler::Stage::Sleep].data(), (draw_time));
//
//draw_time = profiler.GetTime(Profiler::Stage::UpdateLoop);
//ImGui::Text(" ~~ %s % fms ~~ ", profiler.stageNames[(uint8_t)Profiler::Stage::UpdateLoop].data(), (draw_time));
//
//ImGui::End();
