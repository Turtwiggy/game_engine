
//This example creates a window and shows the imgui demo window

//c standard library headers
#include <stdio.h>

//other library headers
#include <glm/glm.hpp>

//your project headers
#include "engine/core/application.hpp"
#include "engine/graphics/render_command.hpp"
#include "engine/tools/profiler.hpp"
#include "engine/ui/profiler_panel.hpp"
using namespace fightingengine;

int main()
{
    uint32_t width = 1366;
    uint32_t height = 768;
    Application app("Hello Application", width, height);
    app.set_fps_limit(60.0f);   
    //app.remove_fps_limit();

    // ---- profiler

    Profiler profiler;
    ProfilerPanel profiler_panel;

    // ---- ui

    bool show_imgui_demo_window = true;

    // ---- app loop

    while(app.is_running())
    {
        profiler.new_frame();
profiler.begin(Profiler::Stage::UpdateLoop);

        app.frame_begin();

        float delta_time_s = app.get_delta_time();

profiler.begin(Profiler::Stage::SdlInput);

        app.poll(); //input events

        //Shutdown app
        if (app.get_input().get_key_down(SDL_KeyCode::SDLK_END))
            app.shutdown();

        //Hold a key
        if (app.get_input().get_key_held(SDL_SCANCODE_F))
            printf("F is being held! \n");

        // Input

        if (app.get_input().get_key_held(SDL_Scancode::SDL_SCANCODE_W))
            printf("w is being held! \n");
        else if (app.get_input().get_key_held(SDL_Scancode::SDL_SCANCODE_S))
            printf("s is being held! \n");

        if (app.get_input().get_key_held(SDL_Scancode::SDL_SCANCODE_A))
            printf("a is being held! \n");
        else if (app.get_input().get_key_held(SDL_Scancode::SDL_SCANCODE_D))
            printf("d is being held! \n");
            
        if (app.get_input().get_key_held(SDL_Scancode::SDL_SCANCODE_SPACE))
            printf("space is being held! \n");
        else if (app.get_input().get_key_held(SDL_Scancode::SDL_SCANCODE_LSHIFT))
            printf("left shift is being held! \n");

profiler.end(Profiler::Stage::SdlInput);
profiler.begin(Profiler::Stage::GameTick);

        // ~~ Game State Tick ~~

        //THIS IS FOR A FIXED GAME TICK
        //seconds_since_last_game_tick += delta_time_in_seconds;
        //while (seconds_since_last_game_tick >= SECONDS_PER_FIXED_TICK)
        //{
        //    //Fixed update
        //    fixed_tick(SECONDS_PER_FIXED_TICK);
        //    seconds_since_last_game_tick -= SECONDS_PER_FIXED_TICK;
        //}

profiler.end(Profiler::Stage::GameTick);
profiler.begin(Profiler::Stage::Render);

        //Example OpenGL usage
        glm::vec4 dark_blue = glm::vec4(0.0f/255.0f, 100.0f/255.0f, 100.0f/255.0f, 1.0f);
        RenderCommand::set_clear_colour(dark_blue);
        RenderCommand::clear();

profiler.end(Profiler::Stage::Render);
profiler.begin(Profiler::Stage::GuiLoop);

        //ImGUI
        app.gui_begin();

            //ImGui demo window
            ImGui::ShowDemoWindow(&show_imgui_demo_window);

            //Show FPS
            ImGui::Begin("FPS");
            ImGui::Text("FPS %f", app.get_raw_fps(delta_time_s));
            ImGui::Text("Average App FPS %f", app.get_average_fps());
            ImGui::End();

            profiler_panel.draw( app, profiler, delta_time_s );

        app.gui_end();

profiler.end(Profiler::Stage::GuiLoop);
profiler.begin(Profiler::Stage::FrameEnd);

        app.frame_end(delta_time_s);

profiler.end(Profiler::Stage::FrameEnd);
profiler.end(Profiler::Stage::UpdateLoop);
    }
    return 0;
}