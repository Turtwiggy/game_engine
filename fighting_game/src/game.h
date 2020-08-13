#pragma once

//Import engine stuff
#include "3d/camera.hpp"
#include "core/data_structures/circular_buffer.h"
#include "graphics/renderer.h"
#include "gui.hpp"
#include "debug/profiler.hpp"
using namespace fightingengine;

//Import game stuff
#include "game_state.hpp"

#include <SDL2/input/input_manager.h>
#include <SDL2/window/game_window.h>

//placeholder systems
//#include "physics/physics_example.hpp""
//#include "audio/audio_player.hpp"
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
    class Game
    {
    public:
        bool process_events(Renderer& r, GameWindow& g_window, Gui& gui, Camera& camera, InputManager& input_manager);

    private:

        //game logic
        void tick(float delta_time, GameState& state, float timer, InputManager& input_maanger, Camera& camera);   
        void fixed_tick(float fixed_delta_time);

        void render
        (
            Profiler& profiler,
            GameWindow& window,
            GameState& state,
            Camera& c,
            Gui& g,
            Renderer& r
        );
        void shutdown(Renderer& r, GameWindow& w);
    };
}
