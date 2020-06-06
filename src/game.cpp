/*

*/

#include "game.hpp"
#include "gui.hpp"

#include <glm/glm.hpp>
#include <spdlog/spdlog.h>
#include "imgui.h"
#include "SDL2/SDL.h"
#include <GL/glew.h>
#include "boost/filesystem.hpp"

#include "graphics/render_command.h"
#include "3d/assimp_obj_loader.h"

#include <cstdint>
#include <string>
#include <filesystem>

using namespace fightinggame;

const std::string kBuildStr = "1";
//const std::string kBuildStr(kGitSHA1Hash, 8);
const std::string kWindowTitle = "fightinggame";

game* game::sInstance = nullptr;

game::game()
    : _eventManager(std::make_unique<event_manager>())
{
    //auto logger = spdlog::basic_logger_mt("default_logger", "logs");
    //spdlog::set_default_logger(logger);

    sInstance = this;

    // create our camera
    _camera = std::make_unique<Camera>(glm::vec3(0.0f, 0.0f, 10.0f));

    _window = std::make_unique<game_window>(kWindowTitle + " [" + kBuildStr + "]", m_width, m_height, display_mode::Windowed);

    _renderer = std::make_unique<renderer>(_window.get(), false);

    _gui = std::make_unique<Gui>();

    //object loader
    std::filesystem::path current_dir = std::filesystem::current_path();
    //current_dir.append("res/models/backpack/backpack.obj");
    current_dir.append("res/models/lizard_wizard/lizard_wizard.obj");
    tempModel = std::make_shared<Model>(current_dir.generic_u8string());
}

bool game::process_window_input_down(const SDL_Event& event)
{
    switch (event.key.keysym.sym)
    {
    case SDLK_ESCAPE:

        shutdown();
        return false;
    case SDLK_f:
        _window->SetFullscreen(!fullscreen);

        if (_window)
        {
            int width, height;
            _window->GetSize(width, height);

            m_width = width;
            m_height = height;
            std::cout << "screen size toggled, w: " << m_width << " h: " << m_height << std::endl;
            render_command::SetViewport(0, 0, m_width, m_height);
        }
        fullscreen = !fullscreen;

        break;
    case SDLK_m:
        _window->ToggleMouseCaptured();
        break;
    }

    return true;
}

bool game::process_events()
{
    SDL_Event e;
    while (SDL_PollEvent(&e))
    {
        //_eventManager->Create<SDL_Event>(e);

        // If gui captures this input, do not propagate
        if (!this->_gui->ProcessEventSdl2(e, _renderer->get_imgui_context()))
        {
            this->_camera->ProcessEvents(e);

            if (e.type == SDL_QUIT)
                return false;
            else if (e.type == SDL_WINDOWEVENT
                && e.window.event == SDL_WINDOWEVENT_CLOSE
                && e.window.windowID == SDL_GetWindowID(_window->GetHandle()))
                return false;

            switch (e.type)
            {
            case SDL_KEYDOWN:
                return process_window_input_down(e);
            }
        }
    }

    return true;
}

//Called X ticks per second
void game::tick(float delta_time, game_state& state)
{
    //printf("ticking frame %i game state time: %f \n", _frameCount, delta_time);

    state.cube_pos = glm::vec3(0.0, 0.0, 0.0);
}

void game::render(game_state& state)
{
    _renderer->new_frame(_window->GetHandle());

    {
        renderer::draw_scene_desc drawDesc;
        drawDesc.view_id = graphics::render_pass::Main;
        drawDesc.height = m_height;
        drawDesc.width = m_width;
        drawDesc.camera = _camera;
        drawDesc.main_character = tempModel;

        _renderer->draw_pass(drawDesc);
    }


    if (_gui->Loop(*this, _renderer->get_imgui_context()))
    {
        running = false;
        return;
    }

    _renderer->end_frame(_window->GetHandle());
}

void game::run()
{
    //// Create profiler
    //_profiler = std::make_unique<Profiler>();

    //// create our camera
    //_camera = std::make_unique<Camera>();
    //auto aspect = _window ? _window->GetAspectRatio() : 1.0f;
    //_camera->SetProjectionMatrixPerspective(70.0f, aspect, 1.0f, 65536.0f);

    if (_window)
    {
        int width, height;
        _window->GetSize(width, height);
        render_command::SetViewport(0, 0, width, height);
    }

    _frameCount = 0;
    while (running)
    {
        // input
        // -----
        running = process_events();
        if (!running) return;

        // Delta Time
        // ----------
        ImGuiIO& io = ImGui::GetIO();
        float delta_time = io.DeltaTime;
        //printf("delta_time %f \n", delta_time);
        _timeSinceLastUpdate += delta_time;

        // Update Systems
        // --------------
        _camera->Update(delta_time);

        // Game Logic Tick
        // ---------------
        while (_timeSinceLastUpdate >= timePerFrame)
        {
            state_previous = state_current;

            tick(timePerFrame, state_current ); //this update's state_current

            _timeSinceLastUpdate -= timePerFrame;
        }

        // Rendering
        // ---------
        render(state_current);

        //lerp between game states
        //const float alpha = _timeSinceLastUpdate / timePerFrame;
        //game_state state_lerped = state_current * alpha + state_previous * ( 1.0 - alpha );
        //render(window, new_state, net_set);

        _frameCount++;
        state_current.frame = _frameCount;
        //printf("frame count: %f", _frameCount);
    }

    //end
}

void game::shutdown()
{
    running = false;

    _renderer->shutdown();
    _window->Close();

    _gui.reset();
    _renderer.reset();
    _window.reset();
    _camera.reset();
    _eventManager.reset();
}
