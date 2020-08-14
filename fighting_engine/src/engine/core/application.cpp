
#include "engine/core/application.h"


#include "SDL2/SDL.h"
#include "SDL2/SDL_events.h"

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

        //renderer = std::make_unique<Renderer>();
        //renderer->init_opengl(*window.get());
        //renderer->init_renderer(m_width, m_height);

        imgui_layer = new ImGui_Manager(window.get());

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
        profiler.Frame();
        profiler.Begin(Profiler::Stage::UpdateLoop);

        input_manager.new_frame();
    }

    void Application::frame_end(float delta_time)
    {
        // FPS Profiling
        // -------------
        fps_buffer.push_back(1.f / delta_time);

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


    void Application::poll()
    {
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

            //TODO RESIZE event

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
    }

    void Application::gui_begin()
    {
        imgui_layer->begin(GetWindow());

        ImGui::Begin("Hello Window");
        ImGui::Text("Hello World");
        ImGui::End();
    }

    void Application::gui_end()
    {
        imgui_layer->end(GetWindow());
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
        //renderer->resize(w, h);
        return false;
    }
}

