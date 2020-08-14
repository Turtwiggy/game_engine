#pragma once

#include "engine/core/application.h"
#include "engine/renderer/render_command.h"
using namespace fightingengine;

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <SDL2/SDL.h>

#undef main
//^thanks SDL2

int main(int argc, char** argv)
{
    int width = 1080;
    int height = 720;
    Application app("Fighting Game!", width, height);

    ////Camera
    //Camera camera = Camera(glm::vec3(0.0f, 0.0f, 10.0f));
    //printf("camera taking up: %s bytes \n", std::to_string(sizeof(camera)));

    ////Model: Cornel Box
    //std::shared_ptr cornel_model = model_manager.load_model("assets/models/cornell_box/CornellBox-Original.obj", "cornell_box");
    //FGObject cornel_box = FGObject(cornel_model);

    ////Model: Cube
    ////std::shared_ptr cube_model = model_manager.load_model("assets/models/lizard_wizard/lizard_wizard.obj", "lizard_wizard");
    //////Cube Objects
    ////FGObject cube_object = FGObject(cube_model);
    ////cube_object.transform.Position = glm::vec3(0.0f, 0.0f, 0.0f);
    ////FGObject cube_object2 = FGObject(cube_model);
    ////cube_object2.transform.Position = glm::vec3(2.0f, 5.0f, -15.0f);

    //////Player Object
    ////FGObject player_object = FGObject(cube_model);
    ////player_object.transform.Position = glm::vec3(-5.0f, 0.5f, -5.0f);

    ////std::vector<std::shared_ptr<FGObject>> cubes;
    ////cubes.push_back(std::make_shared<FGObject>(cube_object));
    ////cubes.push_back(std::make_shared<FGObject>(cube_object2));

    //Renderer (TODO move to renderer)
    RenderCommand::init();
    RenderCommand::set_viewport(0, 0, width, height);

    while (app.is_running())
    {
        app.frame_begin();
        app.poll();
        float delta_time_s = app.get_delta_time();

        if (app.get_input().get_key_down(SDL_KeyCode::SDLK_ESCAPE))
            app.shutdown();

        if (app.get_input().get_key_down(SDL_KeyCode::SDLK_m))
            app.get_window().ToggleMouseCaptured();

        //Cameras
        //c.x += app.input().isKeyDown(APP_KEY_D);
        //c.x += app.input().isKeyDown(APP_KEY_A);
        //c.x += app.inpuisKeyDown(APP_KEY_W);
        //c.x += app.isKeyDown(APP_KEY_S);
        ////camera.update(timestep);

        //Cube
        //graphicsobjects.render(camera);

        //rendering
        RenderCommand::set_clear_colour(glm::vec4(0.2f, 0.6f, 0.2f, 1.0f));
        RenderCommand::clear();
        //app.render(profiler, camera);

        app.gui_begin();

        ImGui::Begin("Hello Window");
        ImGui::Text("Hello World");
        ImGui::End();

        app.gui_end();
        app.frame_end(delta_time_s);
    }

    return 0;
}




////Renderer
//Renderer renderer;
////renderer.init_opengl_and_imgui(window); //do not use opengl before this point
////renderer.init_renderer(m_width, m_height);
////renderer.renderer_impl = std::make_unique<RendererRayTraced>();
////renderer.renderer_impl->init(m_width, m_height);
////printf("renderer taking up: %s bytes \n", std::to_string(sizeof(renderer)).c_str());

////Input Manager
//InputManager input_manager;
//printf("input_manager taking up: %s bytes \n", std::to_string(sizeof(input_manager)).c_str());

//ModelManager model_manager;
//printf("ModelManager taking up: %s bytes \n", std::to_string(sizeof(model_manager)).c_str());


////// Procedural terrain
////std::vector<Texture2D> textures;
////Terrain terrain = Terrain(-5, -5, textures);
////std::shared_ptr terrain_mesh = terrain.get_mesh();
////FGModel tm = FGModel(terrain_mesh, "Procedural Terrain");
////std::shared_ptr terrain_model = std::make_shared<FGModel>(tm);
////FGObject terrain_object = FGObject(terrain_model);

//// Game State
//// ----------
//GameState state_current = GameState(
//    //cubes,
//    //std::make_shared<FGObject>(terrain_object),
//    //std::make_shared<FGObject>(player_object),
//    std::make_shared<FGObject>(cornel_box)
//);


// Rendering
// ---------
//RenderDescriptor descriptor = RenderDescriptor();
//renderer.renderer_impl->draw_pass(descriptor);
//render(profiler, state_current, renderer, camera, window);

//// input
//// -----
//profiler.Begin(Profiler::Stage::SdlInput);
//
//running = process_events(renderer, window, gui, camera, input_manager);
//
//if (!running) { shutdown(renderer, window);  return; }
//
//// User input events
//// -----------
//if (input_manager.get_key_down(SDL_KeyCode::SDLK_m))
//{
//    window.ToggleMouseCaptured();
//}
//if (input_manager.get_key_down(SDL_KeyCode::SDLK_f))
//{
//    window->SetFullscreen(!fullscreen);
//    int width, height;
//    window->GetSize(width, height);
//    std::cout << "screen size toggled, w: " << width << " h: " << height << std::endl;
//    RenderCommand::set_viewport(0, 0, width, height);
//    renderer.renderer_impl->resize(width, height);
//
//    fullscreen = !fullscreen;
//}
//
//
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
//    //Update game state every frame
//    tick(delta_time_in_seconds, state_current, time_since_launch, input_manager, camera);
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
