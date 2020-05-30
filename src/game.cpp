/*

*/

#include "game.h"

//#include "gui.h"

#include <glm/glm.hpp>
#include <spdlog/spdlog.h>
#include "imgui.h"
//#include "imgui_impl_sdl.h"
//#include "imgui_impl_opengl3.h"
#include "SDL2/SDL.h"

#ifdef IMGUI_IMPL_OPENGL_LOADER_GLEW
#include <GL/glew.h>
#endif

#include <cstdint>
#include <string>
#include <memory>

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

    _window = std::make_unique<game_window>(kWindowTitle + " [" + kBuildStr + "]", m_width, m_height, display_mode::Windowed);

    _renderer = std::make_unique<renderer>(_window.get(), bgfx::RendererType::OpenGL, false);

    _eventManager->AddHandler(std::function([this](const SDL_Event& event) {

        running = this->process_events(event);

        //// If gui captures this input, do not propagate
        //if (!this->_gui->ProcessEventSdl2(event))
        //{
        //    //this->_camera->ProcessSDLEvent(event);
        //  running = this->process_events(event);
        //}
        }));

    //state_current.physics.init_physics();

    //Setup Imgui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // setup platform/renderer bindings
    //ImGui_ImplSDL2_InitForOpenGL(_window.get(), gl_context);
    //ImGui_ImplOpenGL3_Init(glsl_version);
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

bool game::update()
{
    //process events
    {
        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            _eventManager->Create<SDL_Event>(e);
        }
    }

    ImGuiIO& io = ImGui::GetIO();
    float delta_time = io.DeltaTime;
    //printf("delta_time %f \n", delta_time);
    _timeSinceLastUpdate += delta_time;


    // Start the Dear ImGui frame
    //ImGui_ImplOpenGL3_NewFrame();
    //ImGui_ImplSDL2_NewFrame(_window.get());
    //ImGui::NewFrame();

    //e.g. if time is 1, we process 60 frames. CPU Spike much.
    while (_timeSinceLastUpdate >= timePerFrame)
    {
        state_previous = state_current;

        //// handle events
        //sf::Event event;
        //process_events(window, event, state_current, timePerFrame);
        //tick(state_current, timePerFrame, event);
        tick(timePerFrame);

        _timeSinceLastUpdate -= timePerFrame;
    }

    const float alpha = _timeSinceLastUpdate / timePerFrame;

    //lerp between game states
    //game_state new_state = state_current * alpha + state_previous * ( 1.0 - alpha );
    //render(window, new_state, net_set);
    //render(window, state_current, net_set);

    //ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    //ImGui_ImplOpenGL3_Shutdown();
    //ImGui_ImplSDL2_Shutdown();
    //ImGui::DestroyContext();
    //SDL_GL_SwapWindow(_window.get());

    return true;
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
        update();   //game logic

        //_renderer->DrawScene(*_meshPack, drawDesc);
        //{
        //    auto section = _profiler->BeginScoped(Profiler::Stage::GuiDraw);
        //    _gui->Draw();
        //}
        //{
        //    auto section = _profiler->BeginScoped(Profiler::Stage::RendererFrame);
        //    _renderer->Frame();
        //}
        // Rendering
        //ImGui::Render();

        {
            renderer::draw_scene_desc drawDesc;
            drawDesc.view_id = graphics::render_pass::Main;
            drawDesc.bgfxDebug = true;
            drawDesc.wireframe = false;
            drawDesc.profile = false;
            drawDesc.height = m_height;
            drawDesc.width = m_width;

            _renderer->draw_scene(drawDesc);
        }
        //_gui->Draw();
        _renderer->frame();

        _frameCount++;
        //printf("frame count: %f", _frameCount);
    }
}

void game::shutdown()
{
    running = false;

    _renderer.reset();
    _window.reset();
    _eventManager.reset();

    //SDL_GL_DeleteContext(gl_context);
    //SDL_DestroyWindow(window);
    SDL_Quit();
}
