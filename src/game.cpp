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
    float lastX = m_width / 2.0f;
    float lastY = m_height / 2.0f;
    bool firstMouse = true;

    _window = std::make_unique<game_window>(kWindowTitle + " [" + kBuildStr + "]", m_width, m_height, display_mode::Windowed);

    _renderer = std::make_unique<renderer>(_window.get(), false);

    _eventManager->AddHandler(std::function([this](const SDL_Event& event) {

        // If gui captures this input, do not propagate
        if (!this->_gui->ProcessEventSdl2(event))
        {
            //this->_camera->ProcessKeyboard(event, delta_time);
            running = this->process_events(event);
        }
        }));

    float scale = 1;
    _gui = Gui::create(_window.get(), graphics::render_pass::ImGui, scale);
}

game::~game()
{
    shutdown();
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
                _renderer->configure_view(graphics::render_pass::Main, width, height);
            }
            fullscreen = !fullscreen;

            break;
        case SDLK_F1:
            //_config.bgfxDebug = !_config.bgfxDebug;
            break;
        }
        break;
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
        _renderer->configure_view(graphics::render_pass::Main, width, height);
    }

    _frameCount = 0;
    while (running)
    {
        //process events
        {
            SDL_Event e;
            while (SDL_PollEvent(&e))
            {
                _eventManager->Create<SDL_Event>(e);
            }
        }
        // ImGui events + prepare
        {
            //    auto guiLoop = _profiler->BeginScoped(Profiler::Stage::GuiLoop);
            if (_gui->Loop(*this, *_renderer))
            {
                running = false;
                return;
            }
        }

        ImGuiIO& io = ImGui::GetIO();
        float delta_time = io.DeltaTime;
        //printf("delta_time %f \n", delta_time);
        _timeSinceLastUpdate += delta_time;

        //e.g. if time is 1, we process 60 frames. CPU Spike much.
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

        {
            renderer::draw_scene_desc drawDesc;
            drawDesc.view_id = graphics::render_pass::Main;
            drawDesc.height = m_height;
            drawDesc.width = m_width;

            _renderer->draw_scene(drawDesc);
        }

        _gui->Draw();

        _renderer->frame(_window->GetHandle());

        _frameCount++;
        //printf("frame count: %f", _frameCount);
    }

    //end
}

void game::shutdown()
{
    running = false;

    _gui.reset();
    _renderer.reset();
    _window.reset();
    _eventManager.reset();

    SDL_Quit();
}
