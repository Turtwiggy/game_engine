#pragma once

#include "engine/core/layer_stack.h"
#include "engine/core/window/game_window.h"
#include "engine/core/data_structures/circular_buffer.h"
#include "engine/events/event.h"
#include "engine/events/app_event.h"
#include "engine/layers/imgui/imgui_layer.h"
#include "engine/util/singleton.h"

#include <memory>
#include <string>

namespace fightingengine {

    class Application : public Singleton<Application>
    {
    public:
        Application(const std::string& name = "Fighting Engine (Default)");
        virtual ~Application();

        void on_event(Event& e);

        void push_layer(Layer* layer);
        void push_overlay(Layer* layer);

        GameWindow& GetWindow() { return *window; }

        void shutdown();
        void run();

    private:
        bool on_window_close(WindowCloseEvent& e);
        bool on_window_resize(WindowResizeEvent& e);

    private:

        std::unique_ptr<GameWindow> window;
        LayerStack layer_stack;
        ImGuiLayer* imgui_layer;

        bool running = true;
        bool fullscreen = false;
        bool minimized = false;

         //FPS settings
        double FPS = 144.0;
        Uint32 MILLISECONDS_PER_FRAME = (Uint32)(1000 / FPS);
        CircularBuffer<float, 100> fps_buffer;

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

    // To be defined in CLIENT
    Application* CreateApplication();

}
