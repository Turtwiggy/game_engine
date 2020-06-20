#include "game.h"

#include "gui.hpp"
#include "graphics/render_command.h"
#include "3d/assimp_obj_loader.h"
#include "entities/transform.h"

#include <glm/glm.hpp>
#include <spdlog/spdlog.h>
#include "imgui.h"
#include "SDL2/SDL.h"
#include <GL/glew.h>
#include "boost/filesystem.hpp"
#include <cstdint>
#include <string>
#include <filesystem>

using namespace fightinggame;

const std::string kBuildStr = "1";
//const std::string kBuildStr(kGitSHA1Hash, 8);
const std::string kWindowTitle = "fightinggame";

game* game::sInstance = nullptr;

bool game::process_window_input_down(const SDL_Event& event, game_window& window)
{
    switch (event.key.keysym.sym)
    {
    case SDLK_ESCAPE:

        return false;
    case SDLK_f:
        window.SetFullscreen(!fullscreen);

        int width, height;
        window.GetSize(width, height);

        m_width = width;
        m_height = height;
        std::cout << "screen size toggled, w: " << m_width << " h: " << m_height << std::endl;
        render_command::SetViewport(0, 0, m_width, m_height);
        
        fullscreen = !fullscreen;

        break;
    case SDLK_m:
        window.ToggleMouseCaptured();
        break;
    }

    return true;
}

bool game::process_events(profiler& p, renderer& r, game_window& g, Gui& gui, Camera& c)
{
    SDL_Event e;
    while (SDL_PollEvent(&e))
    {
        // If gui captures this input, do not propagate
        if (!gui.ProcessEventSdl2(e, r.get_imgui_context()))
        {
            //Update camera when mouse is grabbed
            if(g.IsInputGrabbed())
                c.ProcessEvents(e);

            //Other window events
            if (e.type == SDL_QUIT) {
                return false;
            }
            else if (e.type == SDL_WINDOWEVENT
                && e.window.event == SDL_WINDOWEVENT_CLOSE
                && e.window.windowID == SDL_GetWindowID(g.GetHandle()))
                return false;

            switch (e.type)
            {
            case SDL_KEYDOWN:
                return process_window_input_down(e, g);
            }
        }
    }
    return true;
}

//Physics
void advance_physics(game_state& state, float fixed_delta_time)
{
    state.physics.step_simulation(fixed_delta_time);
}

//Called X ticks per second
void game::tick(float fixed_delta_time, game_state& state, Camera& cam)
{
    //this is a new state
    _frameCount += 1;
    state.frame = _frameCount;
    //advance_physics(state, fixed_delta_time);

    //update state
    printf("ticking frame %i game state time: %f \n", _frameCount, fixed_delta_time);

    state.cube_pos = state.cube_pos - glm::vec3(0.0, 1.0, 0.0);

}

void game::render (
    profiler& profiler,
    game_state& state,
    renderer& r,
    Camera& c,
    Gui& g,
    game_window& window,
    std::vector<std::reference_wrapper<FGTransform>>& models)
{
    {
        profiler.Begin(profiler::Stage::NewFrame);
        r.new_frame(window.GetHandle());
        profiler.End(profiler::Stage::NewFrame);
    }

    {
        profiler.Begin(profiler::Stage::SceneDraw);
        fightinggame::draw_scene_desc drawDesc
        ( 
            models,
            c
        );
        drawDesc.view_id = graphics::render_pass::Main;
        drawDesc.height = m_height;
        drawDesc.width = m_width;

        r.draw_pass(drawDesc);
        profiler.End(profiler::Stage::SceneDraw);
    }

    {
        profiler.Begin(profiler::Stage::GuiLoop);
        if (g.Loop(*this, r.get_imgui_context(), profiler))
        {
            running = false;
            return;
        }
        profiler.End(profiler::Stage::GuiLoop);
    }

    {
        profiler.Begin(profiler::Stage::RenderFrame);
        r.end_frame(window.GetHandle());
        profiler.End(profiler::Stage::RenderFrame);
    }
}

void game::run()
{
    sInstance = this;
    //todo init spdlogger

    //Profiler
    profiler profile;
    printf("profiler taking up: %s bytes \n", std::to_string(sizeof(profiler)).c_str());

    //Window
    game_window window =  game_window
    (
        kWindowTitle + " [" + kBuildStr + "]",
        m_width,
        m_height, 
        display_mode::Windowed
    );
    printf("game window taking up: %s bytes \n", std::to_string(sizeof(game_window)).c_str());

    //Camera
    Camera cam = Camera(glm::vec3(0.0f, 0.0f, 10.0f));
    printf("camera taking up: %s bytes \n", std::to_string(sizeof(Camera)));
    //auto aspect = _window ? _window->GetAspectRatio() : 1.0f;
    //_camera->SetProjectionMatrixPerspective(70.0f, aspect, 1.0f, 65536.0f)

    //Renderer
    renderer rend;
    printf("renderer taking up: %s bytes \n", std::to_string(sizeof(renderer)).c_str());
    rend.init_opengl_and_imgui(window); //do not use opengl before this point

    //Temp obj loader - should be moved in future
    printf("Each model : %s bytes \n", std::to_string(sizeof(FGModel)).c_str());
    const std::string dir = std::string(std::filesystem::current_path().generic_u8string());
    //Lizard wizard
    std::string char_path = dir + "/res/models/lizard_wizard/lizard_wizard.obj";
    FGModel char_model = FGModel(char_path);
    FGTransform char_transform = FGTransform(std::reference_wrapper<FGModel>(char_model));
    char_transform.Scale = glm::vec3(0.f, 0.f, 0.f);
    char_transform.Position = glm::vec3(0.f, 1.f, 0.f);

    //Cube
    std::string cube_path = dir + "/res/models/cube/cube.obj";
    FGModel cube_model = FGModel(cube_path);
    FGTransform cube_transform = FGTransform(std::reference_wrapper<FGModel>(cube_model));
    cube_transform.Scale = glm::vec3(0.f, 0.f, 0.f);
    cube_transform.Position = glm::vec3(0.f, 1.f, 0.f);

    //All Models
    std::vector<std::reference_wrapper<FGTransform>> models;
    models.push_back(char_transform);
    models.push_back(cube_transform);

    rend.init_models_and_shaders(models);

    //ImGui Gui (harhar)
    Gui gui;
    printf("Gui taking up: %s bytes \n", std::to_string(sizeof(Gui)).c_str());

    _frameCount = 0;

    start = now = SDL_GetTicks();
    while (running)
    {
        //Update Profiler
        profile.Frame();
        profile.Begin(profiler::Stage::UpdateLoop);

        // delta time
        // ----------
        //Returns an unsigned 32-bit value representing the 
        //number of milliseconds since the SDL library initialized.
        now = SDL_GetTicks();
        uint32_t delta_time_in_milliseconds = now - prev;
        if (delta_time_in_milliseconds < 0) continue;
        prev = now;
        float delta_time_in_seconds = (delta_time_in_milliseconds / 1000.f);

        // input
        // -----
        profile.Begin(profiler::Stage::SdlInput);
        running = process_events(profile, rend, window, gui, cam);
        profile.End(profiler::Stage::SdlInput);
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
        while (seconds_since_last_game_tick >= SECONDS_PER_GAMETICK)
        {
            //state_previous = state_current;

            tick(SECONDS_PER_GAMETICK, state_current, cam ); //this update's state_current

            seconds_since_last_game_tick -= SECONDS_PER_GAMETICK;
        }

        // Camera
        // ------
        cam.Update(delta_time_in_seconds);

        // Rendering
        // ---------
        render(profile, state_current, rend, cam, gui, window, models);

        //lerp between game states
        //const float alpha = _timeSinceLastUpdate / timePerFrame;
        //game_state state_lerped = state_current * alpha + state_previous * ( 1.0 - alpha );
        //render(window, new_state, net_set);

        float curr_fps = 1.f / delta_time_in_seconds;
        average_fps.add_next( curr_fps  );
        //printf("frame count: %i \n", _frameCount);

        profile.End(profiler::Stage::UpdateLoop);

        //Sleep
        SDL_Delay( MILLISECONDS_PER_FRAME );
    }

    //end
    //shutdown(_renderer, _window);
}

void game::shutdown(renderer& r, game_window& w)
{
    running = false;

    r.shutdown();
    w.Close();
}
