
//This example creates a window and shows the imgui demo window

//c standard library headers
#include <stdio.h>

//other library headers
#include <glm/glm.hpp>

//your project headers
#include "engine/core/application.hpp"
#include "engine/graphics/render_command.hpp"
using namespace fightingengine;

int main()
{
    uint32_t width = 1366;
    uint32_t height = 768;
    Application app("Hello Application", width, height);
    app.set_fps_limit(60.0f);   
    //app.remove_fps_limit();

    bool show_imgui_demo_window = true;

    while(app.is_running())
    {
        app.frame_begin();

        //key input events
        app.poll(); 

        float delta_time_s = app.get_delta_time();

        //Shutdown app
        if (app.get_input().get_key_down(SDL_KeyCode::SDLK_END))
            app.shutdown();

        //Hold a key
        if (app.get_input().get_key_held(SDL_SCANCODE_F))
            printf("F is being held! \n");

        //Example OpenGL usage
        RenderCommand::set_clear_colour(glm::vec4(0.9f, 0.8f, 0.7f, 1.0f));
        RenderCommand::clear();

        //ImGUI
        app.gui_begin();

            //ImGui demo window
            ImGui::ShowDemoWindow(&show_imgui_demo_window);

            //Show FPS
            ImGui::Begin("FPS");
            ImGui::Text("FPS %f", app.get_raw_fps(delta_time_s));
            ImGui::Text("Average App FPS %f", app.get_average_fps());
            ImGui::End();

        app.gui_end();

        //end frame
        app.frame_end(delta_time_s);
    }

}