#pragma once

#include "3d/camera.h"
#include "common/circular_buffer.h"
#include "gui.hpp"
#include "graphics/renderer.h"
#include "tools/profiler.hpp"
#include "window/game_window.h"
#include "util/base.h"

//placeholder systems
#include "physics/physics_example.hpp""
#include "audio/audio_player.hpp"
//#include "networking/networking_common.hpp"
//#include "networking/network_settings.hpp"
//#include "networking/chat_client.hpp"
//#include "networking/chat_server.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/vec3.hpp> 
//#include <entt/entt.hpp>

#include <string>
#include <vector>
#include <memory>
#include <thread>

namespace fightinggame
{
    struct input
    {
        bool key_down = false;
        bool key_up = false;

        SDL_Keycode key;
    };

    struct game_state
    {
        int frame;

        //physics_simulation physics;

        glm::vec3 cube_pos = glm::vec3(0.f, 0.f, 0.f);

        //todo: how to lerp between game states
        //game_state& lerp(const game_state& other, float percent)
        //{
        //    game_state lerped_state = other;
        //    return lerped_state;
        //}
    };

    class game
    {
    public:
        bool process_window_input_down(const SDL_Event& e, game_window& window);
        bool process_events(profiler& p, renderer& r, game_window& g, Gui& gui, Camera& c);
        void run();

        float get_average_fps() { return fps_buffer.average(); }

    private:
        void tick(float delta_time, game_state& state, Camera& camera);    //update game logic
        void render
        (
            profiler& profiler,
            game_state& state,
            renderer& r,
            Camera& c,
            Gui& g,
            game_window& window,
            std::vector<std::reference_wrapper<FGTransform>>& models
        );
        void shutdown(renderer& r, game_window& w);

    private:
        static game* sInstance;

        //delta time metrics
        float FPS = 144.f;
        float MILLISECONDS_PER_FRAME = 1000.f / FPS;
        float SECONDS_PER_FRAME = 1.f / FPS;

        int GAME_TICKS_PER_SECOND = 1;
        float SECONDS_PER_GAMETICK = 1.f / GAME_TICKS_PER_SECOND;

        unsigned int start = 0;
        unsigned int prev = 0;
        unsigned int now = 0;
        //unsigned int next = 0;

        //frame metrics
        uint32_t _frameCount = 0;
        float seconds_since_last_game_tick = 0;
        circular_buffer fps_buffer;

        //game window
        bool mouse_grabbed = true;

        //game_state state_previous;
        game_state state_current;

        bool running = true;
        bool fullscreen = false;

        //physics
        physics_simulation _physics;

        //networking
        std::unique_ptr<std::thread> _networking_thread;

        //audio
        //audio_player audio;

        //networking
        //network_settings net_set;
        //net_set.addrServer.Clear();
        //net_set.addrServer.ParseString("127.0.0.1");
        //net_set.addrServer.m_port = net_set.port;
    };
}
