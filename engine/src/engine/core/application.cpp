
#include "engine/core/application.hpp"
#include "engine/resources/resource_manager.hpp"

#include "SDL2/SDL.h"
#include "SDL2/SDL_events.h"

#include <GL/glew.h>

namespace fightingengine {

    const std::string kBuildStr = "1";
    //const std::string kBuildStr(kGitSHA1Hash, 8);

    Application::Application(const std::string& name, int width, int height)
    {
        //Window
        window = std::make_unique<GameWindow>
        (
            name + " [" + kBuildStr + "]",
            width,
            height,
            display_mode::Windowed
        );

        //renderer = std::make_unique<Renderer>();
        //renderer->init_opengl(*window.get());
        //renderer->init_renderer(m_width, m_height);

        imgui_manager.initialize(window.get());

        running = true;
        start = now = SDL_GetTicks();
    }

    Application::~Application()
    {
        ResourceManager::clear();
        window->Close();
    }

    void Application::shutdown()
    {
        running = false;
    }

    float Application::get_delta_time()
    {
        now = SDL_GetTicks();         //Returns an unsigned 32-bit value representing the number of milliseconds since the SDL library initialized.
        uint32_t delta_time_in_milliseconds = now - prev;
        if (delta_time_in_milliseconds < 0) return 0; prev = now;

        float delta_time_in_seconds = (delta_time_in_milliseconds / 1000.f);
        time_since_launch += delta_time_in_seconds;

        ImGuiIO& io = ImGui::GetIO();
        io.DeltaTime = delta_time_in_seconds;
        //printf("delta_time %f \n", delta_time_in_seconds);

        return delta_time_in_seconds;
    }


    void Application::frame_begin()
    {
        // Update Profiler
        // ---------------
        //profiler.Frame();
        //profiler.Begin(Profiler::Stage::UpdateLoop);

        input_manager.new_frame();
    }

    void Application::frame_end(float delta_time)
    {
        // FPS Profiling
        // -------------
        fps_buffer.push_back(1.f / delta_time);

        SDL_GL_SwapWindow(get_window().GetHandle());

        // Sleep
        // -----
        {
            //profiler.Begin(Profiler::Stage::Sleep);
            SDL_Delay(MILLISECONDS_PER_FRAME);
            //profiler.End(Profiler::Stage::Sleep);
        }

        //profiler.End(Profiler::Stage::UpdateLoop);
    }


    void Application::poll()
    {
        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            if (imgui_manager.ProcessEventSdl2(e))
            {
                //Imgui stole the event
                continue;
            }

            //Events to quit
            if (e.type == SDL_QUIT) {
                on_window_close();
            }

            //https://wiki.libsdl.org/SDL_WindowEvent
            if (e.type == SDL_WINDOWEVENT)
            {
                switch (e.window.event)
                {
                case SDL_WINDOWEVENT_CLOSE:
                    if (e.window.windowID == SDL_GetWindowID(window->GetHandle()))
                    {
                        on_window_close();
                    }
                    break;
                case SDL_WINDOWEVENT_RESIZED:
                    if (e.window.windowID == SDL_GetWindowID(window->GetHandle()))
                    {
                        on_window_resize(e.window.data1, e.window.data2);
                    }
                    break;
                case SDL_WINDOWEVENT_MINIMIZED:
                    if (e.window.windowID == SDL_GetWindowID(window->GetHandle()))
                    {
                        minimized = true;
                    }
                    break;
                case SDL_WINDOWEVENT_FOCUS_GAINED:
                    SDL_Log("Window %d gained keyboard focus \n", e.window.windowID);
                    break;
                case SDL_WINDOWEVENT_FOCUS_LOST:
                    SDL_Log("Window %d lost keyboard focus \n", e.window.windowID);
                    break;
                }
            }

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
            case SDL_MOUSEBUTTONDOWN:
                //mouse specific
            {
                input_manager.add_mouse_down(e.button);
                break;
            }
            }
        }
    }

    void Application::gui_begin()
    {
        imgui_manager.begin(get_window());
    }

    void Application::gui_end()
    {
        imgui_manager.end(get_window());
    }

    void Application::on_window_close()
    {
        printf("application close event recieved");
        running = false;
    }

    void Application::on_window_resize(int w, int h)
    {
        printf("application resize event recieved: %i %i ", w, h);

        if (w == 0 || h == 0)
        {
            minimized = true;
            return;
        }

        minimized = false;
        //renderer->resize(w, h);
    }

}

