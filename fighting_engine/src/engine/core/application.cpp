
#include "application.h"
#include "engine/debug/profiler.hpp""
#include "engine/graphics/renderer.h"

#include "SDL2/SDL.h"
#include "SDL2/SDL_events.h"
#include <engine\core\input\input_manager.h>

namespace fightingengine {

    const std::string kBuildStr = "1";
    //const std::string kBuildStr(kGitSHA1Hash, 8);

    Application::Application(const std::string& name)
    {
        //Window
        int m_width = 1080;
        int m_height = 720;
        window = std::make_unique<GameWindow>
            (
                name + " [" + kBuildStr + "]",
                m_width,
                m_height,
                display_mode::Windowed
                );

        renderer = std::make_unique<Renderer>();
        renderer->init_opengl(*window.get());
        renderer->init_renderer(m_width, m_height);

        imgui_layer = new ImGuiLayer();
        push_overlay(imgui_layer);

        //Profiler
        Profiler profiler;
        printf("profiler taking up: %s bytes \n", std::to_string(sizeof(profiler)).c_str());

        InputManager input_manager;
        printf("input_manager taking up: %s bytes \n", std::to_string(sizeof(input_manager)).c_str());

        running = true;
        start = now = SDL_GetTicks();
    }

    Application::~Application()
    {
        window->Close();
    }

    void Application::shutdown()
    {
        running = false;
    }

    {

        //while (running)
        {
            // Update Profiler
            // ---------------
            profiler.Frame();
            profiler.Begin(Profiler::Stage::UpdateLoop);

            // delta time
            // ----------
            now = SDL_GetTicks();         //Returns an unsigned 32-bit value representing the number of milliseconds since the SDL library initialized.
            uint32_t delta_time_in_milliseconds = now - prev;
            if (delta_time_in_milliseconds < 0) continue; prev = now;
            float delta_time_in_seconds = (delta_time_in_milliseconds / 1000.f);
            time_since_launch += delta_time_in_seconds;

            // Delta Time
            // ----------
            ImGuiIO& io = ImGui::GetIO();
            io.DeltaTime = delta_time_in_seconds;
            //printf("delta_time %f \n", delta_time_in_seconds);

            // Process Events
            // --------------
            input_manager.new_frame();

            SDL_Event e;
            while (SDL_PollEvent(&e))
            {
                if (imgui_layer->ProcessEventSdl2(e))
                {
                    //Imgui stole the event
                    continue;
                }

                //Events to quit
                if (e.type == SDL_QUIT) {
                    on_window_close();
                }
                else if (e.type == SDL_WINDOWEVENT
                    && e.window.event == SDL_WINDOWEVENT_CLOSE
                    && e.window.windowID == SDL_GetWindowID(window->GetHandle())) {
                    on_window_close();
                }

                //if (e.type == SDL_Resze)
                //{
                //    //
                //}

                //Key events
                switch (e.type)
                {
                case SDL_KEYDOWN:
                    //keyboard specific! (need to rework for controllers)
                {
                    SDL_KeyboardEvent key_event = e.key;
                    auto key = key_event.keysym.sym;
                    input_manager.add_button_down(key);
                    break;
                }

                case SDL_KEYUP:
                    //keyboard specific! (need to rework for controllers)
                {
                    SDL_KeyboardEvent key_event = e.key;
                    auto key = key_event.keysym.sym;
                    input_manager.add_button_up(key);
                    break;
                }

                case SDL_MOUSEBUTTONDOWN:
                    //mouse specific
                {
                    input_manager.add_mouse_down(e.button);
                    break;
                }

                case SDL_MOUSEBUTTONUP:
                    //mouse specific
                {
                    input_manager.add_mouse_up(e.button);
                    break;
                }

                }
            }


            // Propagate through layers
            // ------------------------
            /*

            for (Layer* layer : layer_stack)
            {
                layer->on_update(delta_time_in_seconds);
                layer->on_imgui_render();
            }*/

        }

        {
            imgui_layer->begin();

            ImGui::Begin("Hello Window");
            ImGui::Text("Hello World");
            ImGui::End();

            imgui_layer->end();

            // FPS Profiling
            // -------------
            fps_buffer.push_back(1.f / delta_time_in_seconds);

            SDL_GL_SwapWindow(window->GetHandle());

            // Sleep
            // -----
            {
                profiler.Begin(Profiler::Stage::Sleep);
                SDL_Delay(MILLISECONDS_PER_FRAME);
                profiler.End(Profiler::Stage::Sleep);
            }

            profiler.End(Profiler::Stage::UpdateLoop);
        }
        //end
    }

    bool Application::on_window_close()
    {
        printf("application close event recieved");
        running = false;
        return true;
    }

    bool Application::on_window_resize(int w, int h)
    {
        printf("application resize event recieved");

        if (w == 0 || h == 0)
        {
            minimized = true;
            return false;
        }

        minimized = false;
        renderer->resize(w, h);
        return false;
    }
}

////Camera
//Camera camera = Camera(glm::vec3(0.0f, 0.0f, 10.0f));
//printf("camera taking up: %s bytes \n", std::to_string(sizeof(camera)));

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
