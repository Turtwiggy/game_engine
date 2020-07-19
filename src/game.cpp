#include "game.h"

#include "3d/fg_transform.hpp"
#include "input/keyboard_input.h"
#include "graphics/render_command.h"
#include "gui.hpp"

#include <spdlog/spdlog.h>
#include "imgui.h"
#include <GL/glew.h>
#include "boost/filesystem.hpp"

#include <cstdint>
#include <string>
#include <filesystem>

using namespace fightinggame;

const std::string kBuildStr = "1";
//const std::string kBuildStr(kGitSHA1Hash, 8);
const std::string kWindowTitle = "fightinggame";

Game* Game::sInstance = nullptr;

bool Game::process_window_input_down(const SDL_Event& e, GameWindow& window)
{
    switch (e.key.keysym.sym)
    {
    case SDLK_ESCAPE:
        return false;

    case SDLK_f:
        window.SetFullscreen(!fullscreen);

        int width, height;
        window.GetSize(width, height);

        std::cout << "screen size toggled, w: " << width << " h: " << height << std::endl;
        RenderCommand::set_viewport(0, 0, width, height);

        fullscreen = !fullscreen;
        break;
    case SDLK_m:
        window.ToggleMouseCaptured();
        break;
    case SDLK_n:
        printf("n pressed \n");
        break;
    }
    return true;
}

bool Game::process_events(Profiler& p, Renderer& r, GameWindow& g_window, Gui& gui, Camera& camera)
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
                return process_window_input_down(e, g_window);
            }
        }
    }
    return true;
}

void Game::tick(float delta_time_in_seconds, GameState& state, Camera& cam)
{
    //advance_physics(state, fixed_delta_time);

    //update state
    printf("ticking state, delta_time: %f \n", delta_time_in_seconds);

    //state.cube_pos -= glm::vec3(1.0, 0.0, 0.0);
    //printf("cube pos: %f %f %f", state.cube_pos.x, state.cube_pos.y, state.cube_pos.z);
}

void Game::fixed_tick(float fixed_delta_time)
{
    printf("fixed tick");
}

void Game::render(
    Profiler& profiler,
    GameState& state,
    Renderer& rend,
    Camera& c,
    Gui& g,
    GameWindow& window,
    std::vector<std::reference_wrapper<FGTransform>>& models)
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
            models,
            c,
            window
        );
        drawDesc.view_id = graphics::render_pass::Main;
        drawDesc.transforms[1].get().Position = state.cube_pos;

        rend.draw_pass(drawDesc, state);
        profiler.End(Profiler::Stage::SceneDraw);
    }

    //Render GUI
    {
        profiler.Begin(Profiler::Stage::GuiLoop);
        if (g.Loop(*this, rend.get_imgui_context(), profiler))
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
    sInstance = this;

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

    //Temp obj loader - should be moved in future
    printf("Each model : %s bytes \n", std::to_string(sizeof(FGModel)).c_str());
    const std::string dir = std::string(std::filesystem::current_path().generic_u8string());
    //Lizard wizard
    std::string char_path = dir + "/assets/models/lizard_wizard/lizard_wizard.obj";
    FGModel char_model = FGModel(char_path);
    FGTransform char_transform = FGTransform(std::reference_wrapper<FGModel>(char_model));
    char_transform.Scale = glm::vec3(0.f, 0.f, 0.f);
    char_transform.Position = glm::vec3(0.f, 1.f, 0.f);

    //Cube
    std::string cube_path = dir + "/assets/models/cube/cube.obj";
    FGModel cube_model = FGModel(cube_path);
    FGTransform cube_transform = FGTransform(std::reference_wrapper<FGModel>(cube_model));
    cube_transform.Scale = glm::vec3(0.f, 0.f, 0.f);
    cube_transform.Position = glm::vec3(0.f, 1.f, 0.f);

    //All Models
    std::vector<std::reference_wrapper<FGTransform>> models;
    models.push_back(char_transform);
    models.push_back(cube_transform);

    rend.init_models_and_shaders(models);

    //ImGui
    Gui gui;
    printf("Gui taking up: %s bytes \n", std::to_string(sizeof(Gui)).c_str());

    //Keyboard Controller
    keyboard_controller input;

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

        // input
        // -----
        profile.Begin(Profiler::Stage::SdlInput);
        running = process_events(profile, rend, window, gui, cam);
        const Uint8* key_state = SDL_GetKeyboardState(NULL);

        profile.End(Profiler::Stage::SdlInput);
        if (!running) { shutdown(rend, window);  return; }

        // Delta Time
        // ----------
        ImGuiIO& io = ImGui::GetIO();
        io.DeltaTime = delta_time_in_seconds;
        //float delta_time_in_seconds = io.DeltaTime;
        //printf("delta_time %f \n", delta_time_in_seconds);

        seconds_since_last_game_tick += delta_time_in_seconds;

        // Game Logic Tick - X ticks per second
        // ------------------------------------
        while (seconds_since_last_game_tick >= SECONDS_PER_FIXED_TICK)
        {
            //state_previous = state_current;

            fixed_tick(SECONDS_PER_FIXED_TICK);

            seconds_since_last_game_tick -= SECONDS_PER_FIXED_TICK;
        }

        // Update Game State
        // -----------------
        tick(delta_time_in_seconds, state_current, cam);

        // Camera
        // ------
        glm::vec3 move_input = input.get_move_dir(key_state);
        cam.update(move_input, delta_time_in_seconds);

        // Rendering
        // ---------
        render(profile, state_current, rend, cam, gui, window, models);

        //lerp between game states
        //const float alpha = _timeSinceLastUpdate / timePerFrame;
        //game_state state_lerped = state_current * alpha + state_previous * ( 1.0 - alpha );
        //render(window, new_state, net_set);

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
