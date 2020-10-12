#pragma once

#include "engine/core/game_window.hpp"
#include "engine/core/input_manager.hpp"
#include "engine/imgui/imgui_setup.hpp"
//#include "engine/debug/profiler.hpp"

#include <memory>
#include <string>
#include <boost/circular_buffer.hpp>

namespace fightingengine {

    class Application
    {
    public:
        Application(const std::string& name = "Fighting Engine (Default)", int width = 1080, int height = 720);
        virtual ~Application();

        [[nodiscard]] bool is_running();
        void shutdown();

        [[nodiscard]] float get_delta_time();
        void frame_begin();
        void frame_end(const float delta_time);
        void poll(); 
        void gui_begin();
        void gui_end();

        [[nodiscard]] InputManager& get_input();
        [[nodiscard]] ImGui_Manager& get_imgui();
        [[nodiscard]] GameWindow& get_window();

        void set_fps_limit(const double fps);
        [[nodiscard]] float get_average_fps();

    private:

        //window events
        void on_window_close();
        void on_window_resize(int w, int h);

    private:
        std::unique_ptr<GameWindow> window;
        //std::unique_ptr<Renderer> renderer;
        //Profiler profiler;
        InputManager input_manager;
        ImGui_Manager imgui_manager;

        bool running = true;
        bool fullscreen = false;
        bool minimized = false;

        //FPS settings
        double FPS = 60.0;
        Uint32 MILLISECONDS_PER_FRAME = (Uint32)(1000 / FPS);
        boost::circular_buffer<float> fps_buffer;

        //Game's fixed tick
        int FIXED_TICKS_PER_SECOND = 1;
        float SECONDS_PER_FIXED_TICK = 1.f / FIXED_TICKS_PER_SECOND;
        float seconds_since_last_game_tick = 0;

        //Delta time
        unsigned int start = 0;
        unsigned int prev = 0;
        unsigned int now = 0;
        float time_since_launch = 0.0f;
    };
}
