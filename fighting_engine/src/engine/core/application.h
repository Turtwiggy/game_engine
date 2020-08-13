#pragma once

#include "engine/core/layer_stack.h"
#include "engine/core/window/game_window.h"
#include "engine/events/event.h"
#include "engine/events/app_event.h"
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

    private:
        void run();
        bool on_window_close(WindowCloseEvent& e);
        bool on_window_resize(WindowResizeEvent& e);

    private:

        std::unique_ptr<GameWindow> window;
        LayerStack layer_stack;

        bool running = true;
        bool fullscreen = false;

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
