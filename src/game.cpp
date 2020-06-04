/*

*/

#include "game.hpp"
#include "gui.hpp"

#include <glm/glm.hpp>
#include <spdlog/spdlog.h>
#include "imgui.h"
#include "SDL2/SDL.h"

#ifdef IMGUI_IMPL_OPENGL_LOADER_GLEW
#include <GL/glew.h>
#endif

#include "graphics/render_command.h"

#include <cstdint>
#include <string>

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
}

game::~game()
{
    shutdown();
}

bool game::process_input_down(const SDL_Event& event)
{
    switch (event.key.keysym.sym)
    {
    case SDLK_ESCAPE:
        return false;
    case SDLK_f:
        _window->SetFullscreen(!fullscreen);

        if (_window)
        {
            int width, height;
            _window->GetSize(width, height);
            render_command::SetViewport(0, 0, width, height);
        }
        fullscreen = !fullscreen;

        break;
    case SDLK_F1:
        //_config.bgfxDebug = !_config.bgfxDebug;
        break;
    }

    return true;
}

bool game::process_events(const SDL_Event& event)
{
    if (event.type == SDL_QUIT)
        return false;
    else if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(_window->GetHandle()))
        return false;

    switch (event.type)
    {
    case SDL_KEYDOWN:
        return process_input_down(event);
    //case SDL_KEYUP:
        //process_input_up(event);
    }

    return true;
}

//Called X ticks per second
void game::tick(float delta_time)
{
    //printf("ticking frame %i game state time: %f \n", _frameCount, delta_time);
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
        ImGuiIO& io = ImGui::GetIO();
        float delta_time = io.DeltaTime;
        //printf("delta_time %f \n", delta_time);
        _timeSinceLastUpdate += delta_time;

        //process input events
        {
            SDL_Event e;
            while (SDL_PollEvent(&e))
            {
                //_eventManager->Create<SDL_Event>(e);

                // If gui captures this input, do not propagate
                if (!this->_gui->ProcessEventSdl2(e, _renderer->get_imgui_context()))
                {
                    this->_camera->ProcessEvents(e, delta_time);

                    running = this->process_events(e);
                }
            }
        }

        //e.g. Game Logic Tick
        while (_timeSinceLastUpdate >= timePerFrame)
        {
            state_previous = state_current;

            tick(timePerFrame /*, state_current */);

            _timeSinceLastUpdate -= timePerFrame;
        }

        const float alpha = _timeSinceLastUpdate / timePerFrame;

        //lerp between game states
        //game_state new_state = state_current * alpha + state_previous * ( 1.0 - alpha );
        //render(window, new_state, net_set);

        // camera
        //glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        //glm::mat4 view = camera.GetViewMatrix();
        //glm::mat4 view_projection = projection * view;

        _renderer->new_frame(_window->GetHandle());

        {
            renderer::draw_scene_desc drawDesc;
            drawDesc.view_id = graphics::render_pass::Main;
            drawDesc.height = m_height;
            drawDesc.width = m_width;
            drawDesc.camera = _camera;

            _renderer->draw_pass(drawDesc);
        }


        if (_gui->Loop(*this, _renderer->get_imgui_context()))
        {
            running = false;
            return;
        }

        _renderer->end_frame(_window->GetHandle());

        _frameCount++;
        //printf("frame count: %f", _frameCount);
    }

    //end
}

void game::shutdown()
{
    running = false;

    _renderer.reset();
    _gui.reset();
    _window.reset();
    _eventManager.reset();

    SDL_Quit();
}
