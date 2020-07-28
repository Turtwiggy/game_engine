#include "game.h"

#include "3d/fg_model_manager.h"
#include "3d/fg_object.hpp"
#include "modules/terrain/terrain.hpp"

#include "graphics/render_command.h"
#include "gui.hpp"

#include <GL/glew.h>

#include <cstdint>
#include <string>

using namespace fightinggame;

const std::string kBuildStr = "1";
//const std::string kBuildStr(kGitSHA1Hash, 8);
const std::string kWindowTitle = "fightinggame";

bool Game::process_events(Renderer& r, GameWindow& g_window, Gui& gui, Camera& camera, InputManager& input_manager)
{
    SDL_Event e;
    while (SDL_PollEvent(&e))
    {
        // If gui captures this input, do not propagate
        if (!gui.ProcessEventSdl2(e, r.get_imgui_context()))
        {
            //Update camera when mouse is grabbed
            if (g_window.IsInputGrabbed())
                camera.process_events(e);

            //Other window events
            if (e.type == SDL_QUIT)
                return false;
            else if (e.type == SDL_WINDOWEVENT
                && e.window.event == SDL_WINDOWEVENT_CLOSE
                && e.window.windowID == SDL_GetWindowID(g_window.GetHandle()))
                return false;

            switch (e.type)
            {
            case SDL_KEYDOWN:
                //keyboard specific! (need to rework for controllers)
                {
                    SDL_KeyboardEvent key_event = e.key;
                    auto key = key_event.keysym.sym;
                    input_manager.add_button_down(key);
                }
                return true;

            case SDL_KEYUP:
                //keyboard specific! (need to rework for controllers)
                {
                    SDL_KeyboardEvent key_event = e.key;
                    auto key = key_event.keysym.sym;
                    input_manager.add_button_up(key);
                }
                return true;

            case SDL_MOUSEBUTTONDOWN:
                //mouse specific
                {
                    input_manager.add_mouse_down(e.button);
                }
                return true;

            case SDL_MOUSEBUTTONUP:
                //mouse specific
                {
                    input_manager.add_mouse_up(e.button);
                }
                return true;

            }
        }
    }
    return true;
}

void Game::tick(float delta_time_in_seconds, GameState& state, float timer, InputManager& input_manager, Camera& camera)
{
    //printf("ticking state, delta_time: %f \n", delta_time_in_seconds);

    //std::shared_ptr<FGObject> cube0 = state.cubes[0];
    //cube0->transform.Position.x = 5.0f + glm::sin(timer);
    //cube0->transform.Position.y = 0.0f;
    //cube0->transform.Position.z = 5.0f + glm::cos(timer);

    //const float pi = 3.14;
    //const float frequency = 0.3f; // Frequency in Hz
    //float bouncy_val = 0.5 * (1.0 + sin(2.0 * pi * frequency * timer));

    //cube0->transform.Scale.x = glm::max(0.3f, bouncy_val);
    //cube0->transform.Scale.y = 1.0f;
    //cube0->transform.Scale.z = glm::max(0.3f, bouncy_val);
    ////printf("cube pos: %f %f %f", state.cube_pos.x, state.cube_pos.y, state.cube_pos.z);
    ////printf("lerp sin_val: %f x: %f z: %f \n ", bouncy_val);

    ////Player Cube
    //std::shared_ptr<FGObject> player_cube = state.player;
    //if (input_manager.get_key_held(SDLK_UP))
    //{
    //    player_cube->transform.Position.z += 1.0f * delta_time_in_seconds;
    //}
    //if (input_manager.get_key_held(SDLK_DOWN))
    //{
    //    player_cube->transform.Position.z -= 1.0f * delta_time_in_seconds;
    //}
    //if (input_manager.get_key_held(SDLK_LEFT))
    //{
    //    player_cube->transform.Position.x -= 1.0f * delta_time_in_seconds;
    //}
    //if (input_manager.get_key_held(SDLK_RIGHT))
    //{
    //    player_cube->transform.Position.x += 1.0f * delta_time_in_seconds;
    //}

    //Camera
    //------
    camera.process_users_input(input_manager);
    camera.update(delta_time_in_seconds);
}

void Game::fixed_tick(float fixed_delta_time_in_seconds)
{
    //printf("fixed tick");
    //advance_physics(state, fixed_delta_time);
}

void Game::render(
    Profiler& profiler,
    GameState& state,
    Renderer& rend,
    Camera& camera,
    Gui& gui,
    GameWindow& window,
    bool hdr,
    float exposure )
{
    //Begin Frame
    {
        profiler.Begin(Profiler::Stage::NewFrame);
        rend.new_frame(window.GetHandle());
        profiler.End(Profiler::Stage::NewFrame);
    }

    //Main rendering
    {
        profiler.Begin(Profiler::Stage::MainDraw);

        fightinggame::draw_scene_desc drawDesc
        (
            camera,
            window,
            hdr,
            exposure
        );

        rend.draw_pass
        (
            drawDesc,
            state
        );
        profiler.End(Profiler::Stage::MainDraw);
    }

    //Render GUI
    {
        profiler.Begin(Profiler::Stage::GuiLoop);
        if (gui.Loop(*this, profiler))
        {
            running = false;
            return;
        }
        profiler.End(Profiler::Stage::GuiLoop);
    }

    //End Frame
    {
        profiler.Begin(Profiler::Stage::EndFrame);
        rend.end_frame(window.GetHandle());
        profiler.End(Profiler::Stage::EndFrame);
    }
}

void Game::run()
{
    //Profiler
    Profiler profiler;
    printf("profiler taking up: %s bytes \n", std::to_string(sizeof(profiler)).c_str());

    //Window
    int m_width = 1080;
    int m_height = 720;
    GameWindow window = GameWindow
    (
        kWindowTitle + " [" + kBuildStr + "]",
        m_width,
        m_height,
        display_mode::Windowed
    );
    printf("game window taking up: %s bytes \n", std::to_string(sizeof(window)).c_str());

    //Camera
    Camera camera = Camera(glm::vec3(0.0f, 0.0f, 10.0f));
    printf("camera taking up: %s bytes \n", std::to_string(sizeof(camera)));
    //auto aspect = _window ? _window->GetAspectRatio() : 1.0f;
    //_camera->SetProjectionMatrixPerspective(70.0f, aspect, 1.0f, 65536.0f)

    //Renderer
    Renderer renderer;
    renderer.init_opengl_and_imgui(window); //do not use opengl before this point
    renderer.init_renderer(m_width, m_height);
    printf("renderer taking up: %s bytes \n", std::to_string(sizeof(renderer)).c_str());

    //Input Manager
    InputManager input_manager;
    printf("input_manager taking up: %s bytes \n", std::to_string(sizeof(input_manager)).c_str());

    ModelManager model_manager;
    printf("ModelManager taking up: %s bytes \n", std::to_string(sizeof(model_manager)).c_str());

    ////Model: Cornel Box
    //std::shared_ptr cornel_model = model_manager.load_model("assets/models/cornell_box/CornellBox-Original.obj", "cornell_box");
    //FGObject cornel_box = FGObject(cornel_model);

    ////Model: Cube
    //std::shared_ptr cube_model = model_manager.load_model("assets/models/lizard_wizard/lizard_wizard.obj", "lizard_wizard");
    ////Cube Objects
    //FGObject cube_object = FGObject(cube_model);
    //cube_object.transform.Position = glm::vec3(0.0f, 0.0f, 0.0f);
    //FGObject cube_object2 = FGObject(cube_model);
    //cube_object2.transform.Position = glm::vec3(2.0f, 5.0f, -15.0f);

    ////Player Object
    //FGObject player_object = FGObject(cube_model);
    //player_object.transform.Position = glm::vec3(-5.0f, 0.5f, -5.0f);

    //std::vector<std::shared_ptr<FGObject>> cubes;
    //cubes.push_back(std::make_shared<FGObject>(cube_object));
    //cubes.push_back(std::make_shared<FGObject>(cube_object2));

    //// Procedural terrain
    //std::vector<Texture2D> textures;
    //Terrain terrain = Terrain(-5, -5, textures);
    //std::shared_ptr terrain_mesh = terrain.get_mesh();
    //FGModel tm = FGModel(terrain_mesh, "Procedural Terrain");
    //std::shared_ptr terrain_model = std::make_shared<FGModel>(tm);
    //FGObject terrain_object = FGObject(terrain_model);

    // Game State
    // ----------
    GameState state_current = GameState(
        //cubes,
        //std::make_shared<FGObject>(terrain_object),
        //std::make_shared<FGObject>(player_object),
        //std::make_shared<FGObject>(cornel_box)
    );

    //ImGui
    Gui gui;
    printf("Gui taking up: %s bytes \n", std::to_string(sizeof(gui)).c_str());

    running = true;
    start = now = SDL_GetTicks();
    while (running)
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
        timer += delta_time_in_seconds;

        // input
        // -----
        profiler.Begin(Profiler::Stage::SdlInput);

        input_manager.new_frame();
        running = process_events(renderer, window, gui, camera, input_manager);
        if (!running) { shutdown(renderer, window);  return; }

        // User input events
        // -----------
        if (input_manager.get_key_down(SDL_KeyCode::SDLK_m))
        {
            window.ToggleMouseCaptured();
        }
        if (input_manager.get_key_down(SDL_KeyCode::SDLK_f))
        {
            window.SetFullscreen(!fullscreen);
            int width, height;
            window.GetSize(width, height);
            std::cout << "screen size toggled, w: " << width << " h: " << height << std::endl;
            RenderCommand::set_viewport(0, 0, width, height);
            renderer.screen_size_changed(width, height);
            
            fullscreen = !fullscreen;
        }
        if (input_manager.get_key_down(SDL_KeyCode::SDLK_ESCAPE))
        {
            shutdown(renderer, window);  return;
        }
        if (input_manager.get_mouse_lmb_held())
        {
            //printf("\nlmb held");
        }

        if (input_manager.get_key_down(SDL_KeyCode::SDLK_h))
            hdr = true;
        if (input_manager.get_key_up(SDL_KeyCode::SDLK_h))
            hdr = false;
        
        if (input_manager.get_key_held(SDL_KeyCode::SDLK_e))
        {
            exposure += 0.001f;

            if (exposure > 1.0f)
                exposure = 1.0f;
        }
        if (input_manager.get_key_held(SDL_KeyCode::SDLK_q))
        {
            exposure -= 0.001f;

            if (exposure < 0.0f)
                exposure = 0.0f;
        }

        profiler.End(Profiler::Stage::SdlInput);

        // Delta Time
        // ----------
        ImGuiIO& io = ImGui::GetIO();
        io.DeltaTime = delta_time_in_seconds;
        //printf("delta_time %f \n", delta_time_in_seconds);

        // Game State Tick
        // ---------------
        {
            profiler.Begin(Profiler::Stage::GameTick);

            seconds_since_last_game_tick += delta_time_in_seconds;

            while (seconds_since_last_game_tick >= SECONDS_PER_FIXED_TICK)
            {
                //Fixed update
                fixed_tick(SECONDS_PER_FIXED_TICK);

                seconds_since_last_game_tick -= SECONDS_PER_FIXED_TICK;
            }

            //Update game state every frame
            tick(delta_time_in_seconds, state_current, timer, input_manager, camera);

            profiler.End(Profiler::Stage::GameTick);
        }

        // Rendering
        // ---------
        render(profiler, state_current, renderer, camera, gui, window, hdr, exposure);

        // FPS Profiling
        // -------------
        fps_buffer.push_back(1.f / delta_time_in_seconds);

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

void Game::shutdown(Renderer& r, GameWindow& w)
{
    running = false;

    r.shutdown();
    w.Close();
}
