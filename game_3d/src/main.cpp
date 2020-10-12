#define _CRT_SECURE_NO_WARNINGS
#define IMGUI_IMPL_OPENGL_LOADER_GLEW

#include "engine/core/application.hpp"
//#include "engine/resources/model_manager.hpp"
#include "engine/3d/camera.hpp"
#include "engine/3d/renderer/renderer_ray_traced.hpp"
#include "engine/renderer/render_command.hpp"
#include "engine/renderer/shader.hpp"
#include "engine/geometry/triangle.hpp"
#include "engine/tools/profiler.hpp"
using namespace fightingengine;

#include "panels/scene_hierarchy_panel.hpp"
#include "game_state.hpp"
using namespace game_3d;

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <SDL2/SDL.h>
#undef main //thanks SDL2?

#include <memory>
#include <vector>
#include <string_view>

int main(int argc, char** argv)
{
    int width = 1366;
    int height = 768;
    Application app("Fighting Game!", width, height);
    app.set_fps_limit(60.0);

    //Camera
    Camera camera = Camera (
        glm::vec3(0.0f, 0.0f, 1.0f), 
        glm::vec3(0.0f, 1.0f, 0.0f) );

    //Create world (for now, just spheres)
    std::vector<Sphere> world = create_world();
    
    //ModelManager model_manager;
    GameState state;
    //state.init(model_manager);
    RandomState rnd;

    RendererRayTraced renderer;
    renderer.init(width, height);

    //UI
    auto default_scene = std::make_shared<Scene>();
    SceneHierarchyPanel scene_panel;
    scene_panel.set_context(default_scene);

    Profiler profiler;

    //Testing stuff
    std::string_view s1{"hello world 1"};
    std::string_view s2{"hello world 2"};
    std::string_view string_to_display{ s1 };
    bool new_grab = false;

    float timer = 0.0f;
    while (app.is_running())
    {
        profiler.new_frame();
        profiler.begin(Profiler::Stage::UpdateLoop);
        app.frame_begin();
        app.poll();

        float delta_time_s = app.get_delta_time();
        timer += delta_time_s;

        { // ~~ Input ~~
            profiler.begin(Profiler::Stage::SdlInput);

            //Shutdown app
            if (app.get_input().get_key_down(SDL_KeyCode::SDLK_END))
                app.shutdown();

            //Toggle mouse capture
            if (app.get_input().get_key_down(SDL_KeyCode::SDLK_m))
            {
                new_grab = app.get_window().IsInputGrabbed();

                if(!new_grab)
                {
                    app.get_window().CaptureMouse();
                    new_grab = true;
                }
                else
                {
                    app.get_window().ReleaseMouse();
                    new_grab = false;
                }

                printf("(App) Mouse grabbed? : %d \n", new_grab);
            }

            //Fullscreen
            if (app.get_input().get_key_down(SDL_KeyCode::SDLK_f))
            {
                if(app.get_window().IsFullscreen())
                    app.get_window().SetFullscreen(false);
                else
                    app.get_window().SetFullscreen(true);

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
                if(new_grab)
                {
                    SDL_GetRelativeMouseState(&x, &y); //throw away this huge jumping value
                    new_grab = false;
                }
                else
                {
                    SDL_GetRelativeMouseState(&x, &y);
                    //printf("relative movement: %i %i \n", x, y);
                    camera.process_mouse_movement(x, y, true);
                }
            }

            profiler.end(Profiler::Stage::SdlInput);
        }

        { // ~~ Game State Tick ~~
            profiler.begin(Profiler::Stage::GameTick);
        
            //THIS IS FOR A FIXED GAME TICK
            //seconds_since_last_game_tick += delta_time_in_seconds;
            //while (seconds_since_last_game_tick >= SECONDS_PER_FIXED_TICK)
            //{
            //    //Fixed update
            //    fixed_tick(SECONDS_PER_FIXED_TICK);
            //    seconds_since_last_game_tick -= SECONDS_PER_FIXED_TICK;
            //}

            profiler.end(Profiler::Stage::GameTick);
        }

        { // ~~ Rendering ~~
            profiler.begin(Profiler::Stage::Render);

            state.render(renderer, camera, app.get_window(), timer);
            default_scene->on_update(delta_time_s);

            profiler.end(Profiler::Stage::Render);
        }

        { // ~~ GUI ~~
            profiler.begin(Profiler::Stage::GuiLoop);

            app.gui_begin();
            scene_panel.on_imgui_render();

            bool demo_window = true;
            ImGui::ShowDemoWindow(&demo_window);

            ImGui::Begin("Hello Window");

            ImGui::Text(std::string(string_to_display).c_str());

            ImGui::End();

            ImGui::Begin("Profiler");

            float average_fps = app.get_average_fps();
            float raw_fps = app.get_raw_fps(delta_time_s);
            ImGui::Text("Average FPS: %f", average_fps);

            // Fill an array of contiguous float values to plot
            // Tip: If your float aren't contiguous but part of a structure, you can pass a pointer to your first float
            // and the sizeof() of your structure in the "stride" parameter.
            static bool animate = true;
            ImGui::Checkbox("Animate", &animate);

            static float values[90] = {};
            static int values_offset = 0;
            static double refresh_time = 0.0;
            if (!animate || refresh_time == 0.0)
                refresh_time = ImGui::GetTime();
            while (refresh_time < ImGui::GetTime()) // Create data at fixed 60 Hz rate for the demo
            {
                values[values_offset] = raw_fps;
                values_offset = (values_offset + 1) % IM_ARRAYSIZE(values);
                refresh_time += 1.0f / 60.0f;
            }

            {
                float average = 0.0f;
                for (int n = 0; n < IM_ARRAYSIZE(values); n++)
                    average += values[n];
                average /= (float)IM_ARRAYSIZE(values);
                char overlay[32];
                sprintf(overlay, "avg %f", average);
                ImGui::PlotLines("Lines", values, IM_ARRAYSIZE(values), values_offset, overlay, 0.0f, 144.0f, ImVec2(0, 40.0f));
            }
            
            ImGui::Columns(1);
            
            float time = profiler.get_average_time(Profiler::Stage::SdlInput);
            ImGui::Text("%s %f ms", profiler.stageNames[(uint8_t)Profiler::Stage::SdlInput].data(), (time));
            
            time = profiler.get_average_time(Profiler::Stage::GameTick);
            ImGui::Text("%s %f ms", profiler.stageNames[(uint8_t)Profiler::Stage::GameTick].data(), (time));

            time = profiler.get_average_time(Profiler::Stage::Render);
            ImGui::Text("%s %f ms", profiler.stageNames[(uint8_t)Profiler::Stage::Render].data(), (time));
            
            time = profiler.get_average_time(Profiler::Stage::GuiLoop);
            ImGui::Text("%s %f ms", profiler.stageNames[(uint8_t)Profiler::Stage::GuiLoop].data(), (time));
            
            time = profiler.get_average_time(Profiler::Stage::FrameEnd);
            ImGui::Text("%s %f ms", profiler.stageNames[(uint8_t)Profiler::Stage::FrameEnd].data(), (time));
            
            time = profiler.get_average_time(Profiler::Stage::UpdateLoop);
            ImGui::Text("~~ %s %f ms ~~", profiler.stageNames[(uint8_t)Profiler::Stage::UpdateLoop].data(), (time));
            
            ImGui::End();

            app.gui_end();

            profiler.end(Profiler::Stage::GuiLoop);
        }

        profiler.begin(Profiler::Stage::FrameEnd);

        app.frame_end(delta_time_s);

        profiler.end(Profiler::Stage::FrameEnd);
        profiler.end(Profiler::Stage::UpdateLoop);
    }

    return 0;
}