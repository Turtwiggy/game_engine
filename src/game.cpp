#include "game.h"

#include "3d/fg_model_manager.h"
#include <3d/fg_object.hpp>

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
            }
        }
    }
    return true;
}

void Game::tick(float delta_time_in_seconds, GameState& state, float timer)
{
    //printf("ticking state, delta_time: %f \n", delta_time_in_seconds);

    state.cube_pos.x = glm::sin(1.0f);
    state.cube_pos.y = 2.0f;
    state.cube_pos.z = glm::cos(1.0f);

    //state.cube_pos -= glm::vec3(1.0, 0.0, 0.0);
    //printf("cube pos: %f %f %f", state.cube_pos.x, state.cube_pos.y, state.cube_pos.z);
}

void Game::fixed_tick(float fixed_delta_time_in_seconds)
{
    printf("fixed tick");
    //advance_physics(state, fixed_delta_time);
}

void Game::render(
    Profiler& profiler,
    GameState& state,
    Renderer& rend,
    Camera& camera,
    Gui& gui,
    GameWindow& window,
    FGObject& cube)
{
    //Begin Frame
    {
        profiler.Begin(Profiler::Stage::NewFrame);
        rend.new_frame(window.GetHandle());
        profiler.End(Profiler::Stage::NewFrame);
    }

    //Main rendering
    {
        profiler.Begin(Profiler::Stage::SceneDraw);

        //set cube to pos from gamestate
        //models[1].get().Position = state.cube_pos;

        fightinggame::draw_scene_desc drawDesc
        (
            cube,
            camera,
            window
        );
        drawDesc.view_id = graphics::render_pass::Main;
        drawDesc.object.transform->Position = state.cube_pos;

        rend.draw_pass
        (
            drawDesc,
            state
        );
        profiler.End(Profiler::Stage::SceneDraw);
    }

    //Render GUI
    {
        profiler.Begin(Profiler::Stage::GuiLoop);
        if (gui.Loop(*this, rend.get_imgui_context(), profiler))
        {
            running = false;
            return;
        }
        profiler.End(Profiler::Stage::GuiLoop);
    }

    //End Frame
    {
        profiler.Begin(Profiler::Stage::RenderFrame);
        rend.end_frame(window.GetHandle());
        profiler.End(Profiler::Stage::RenderFrame);
    }
}

void Game::run()
{
    //Profiler
    Profiler profile;
    printf("profiler taking up: %s bytes \n", std::to_string(sizeof(Profiler)).c_str());

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
    printf("game window taking up: %s bytes \n", std::to_string(sizeof(GameWindow)).c_str());

    //Camera
    Camera cam = Camera(glm::vec3(0.0f, 0.0f, 10.0f));
    printf("camera taking up: %s bytes \n", std::to_string(sizeof(Camera)));
    //auto aspect = _window ? _window->GetAspectRatio() : 1.0f;
    //_camera->SetProjectionMatrixPerspective(70.0f, aspect, 1.0f, 65536.0f)

    //Renderer
    Renderer rend;
    printf("renderer taking up: %s bytes \n", std::to_string(sizeof(Renderer)).c_str());
    rend.init_opengl_and_imgui(window); //do not use opengl before this point
    rend.init_shaders();

    //Input Manager
    InputManager input_manager;
    printf("input_manager taking up: %s bytes \n", std::to_string(sizeof(InputManager)).c_str());

    ResourceManager resource_manager;
    printf("ResourceManager taking up: %s bytes \n", std::to_string(sizeof(ResourceManager)).c_str());

    ModelManager model_manager;
    printf("ModelManager taking up: %s bytes \n", std::to_string(sizeof(ModelManager)).c_str());

    //Lizard Wizard
    std::shared_ptr lizard_model = model_manager.load_model("assets/models/lizard_wizard/lizard_wizard.obj", "lizard wizard");
    std::shared_ptr lizard_transform = std::make_shared<FGTransform>();
    lizard_transform->Scale = glm::vec3(0.f, 0.f, 0.f);
    lizard_transform->Position = glm::vec3(0.f, 1.f, 0.f);
    FGObject lizard_object = FGObject(lizard_model, lizard_transform);

    //Cube
    std::shared_ptr cube_model = model_manager.load_model("assets/models/cube/cube.obj", "cube");
    std::shared_ptr cube_transform = std::make_shared<FGTransform>();
    cube_transform->Scale = glm::vec3(0.f, 0.f, 0.f);
    cube_transform->Position = glm::vec3(0.f, 1.f, 0.f);
    FGObject cube_object = FGObject(cube_model, cube_transform);

    //ImGui
    Gui gui;
    printf("Gui taking up: %s bytes \n", std::to_string(sizeof(Gui)).c_str());

    running = true;
    _frameCount = 0;
    start = now = SDL_GetTicks();
    while (running)
    {
        //Update Profiler
        profile.Frame();
        profile.Begin(Profiler::Stage::UpdateLoop);

        // delta time
        // ----------
        now = SDL_GetTicks();         //Returns an unsigned 32-bit value representing the number of milliseconds since the SDL library initialized.
        uint32_t delta_time_in_milliseconds = now - prev;
        if (delta_time_in_milliseconds < 0) continue; prev = now;
        float delta_time_in_seconds = (delta_time_in_milliseconds / 1000.f);
        timer += delta_time_in_seconds;

        // input
        // -----
        profile.Begin(Profiler::Stage::SdlInput);
        input_manager.new_frame();
        running = process_events(rend, window, gui, cam, input_manager);
        profile.End(Profiler::Stage::SdlInput);
        if (!running) { shutdown(rend, window);  return; }

        // User events
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
            fullscreen = !fullscreen;
        }
        if (input_manager.get_key_down(SDL_KeyCode::SDLK_ESCAPE))
        {
            shutdown(rend, window);  return;
        }

        // Delta Time
        // ----------
        ImGuiIO& io = ImGui::GetIO();
        io.DeltaTime = delta_time_in_seconds;
        //printf("delta_time %f \n", delta_time_in_seconds);

        // Game Logic Tick - X ticks per second
        // ------------------------------------
        seconds_since_last_game_tick += delta_time_in_seconds;

        while (seconds_since_last_game_tick >= SECONDS_PER_FIXED_TICK)
        {
            //state_previous = state_current;

            fixed_tick(SECONDS_PER_FIXED_TICK);

            seconds_since_last_game_tick -= SECONDS_PER_FIXED_TICK;
        }

        // Update Game State
        // -----------------
        tick(delta_time_in_seconds, state_current, timer);

        // Camera
        // ------
        cam.process_users_input(input_manager);
        cam.update(delta_time_in_seconds);

        // Rendering
        // ---------
        render(profile, state_current, rend, cam, gui, window, cube_object);

        // FPS Profiling
        // -------------
        float curr_fps = 1.f / delta_time_in_seconds;
        fps_buffer.add_next(curr_fps);
        //printf("frame count: %i \n", _frameCount);

        profile.End(Profiler::Stage::UpdateLoop);

        //Sleep
        SDL_Delay(MILLISECONDS_PER_FRAME);
    }

    //end
    //shutdown(_renderer, _window);
}

void Game::shutdown(Renderer& r, GameWindow& w)
{
    running = false;

    r.shutdown();
    w.Close();
}

////Physics
//void advance_physics(GameState& state, float fixed_delta_time)
//{
//    //state.physics.step_simulation(fixed_delta_time);
//}

//lerp between game states
//const float alpha = _timeSinceLastUpdate / timePerFrame;
//game_state state_lerped = state_current * alpha + state_previous * ( 1.0 - alpha );
//render(window, new_state, net_set);
