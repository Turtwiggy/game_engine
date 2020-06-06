#pragma once

#include <string>
#include <vector>
#include <memory>
#include <thread>

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/vec3.hpp> 
//#include <entt/entt.hpp>

#include "gui.hpp"
#include "3d/camera.h"
#include "common/event_manager.h"
#include "graphics/renderer.h"
#include "tools/profiler.hpp"
#include "window/game_window.h"

//placeholder systems
#include "physics/physics_example.hpp""
#include "audio/audio_player.hpp"
#include "networking/networking_common.hpp"
#include "networking/network_settings.hpp"
#include "networking/chat_client.hpp"
#include "networking/chat_server.hpp"

namespace fightinggame
{
    struct game_state
    {
        int frame;

        physics_simulation physics;

        glm::vec3 cube_pos;

        game_state operator*(const game_state& r) const {

            return *this;
        }
    };

    class game
    {
    public:
        game();

        bool process_window_input_down(const SDL_Event& event);
        bool process_events();
        void run();

        game_window* GetWindow() { return _window.get(); }
        [[nodiscard]] const game_window& GetWindow() const { return *_window; }
        [[nodiscard]] Camera& GetCamera() const { return *_camera; }
        [[nodiscard]] profiler& GetProfiler() const { return *_profiler; }

    private:
        void tick(float delta_time, game_state& state);    //update game logic
        void render(game_state& state);
        void shutdown();

    private:
        static game* sInstance;

        std::unique_ptr<game_window> _window;
        std::unique_ptr<renderer> _renderer;
        std::unique_ptr<Gui> _gui;
        std::shared_ptr<Camera> _camera;
        std::unique_ptr<profiler> _profiler;
        std::unique_ptr<event_manager> _eventManager;

        uint32_t _frameCount = 0;
        const float timePerFrame = 1.f / 60.f;
        float _timeSinceLastUpdate = 0;
        bool mouse_grabbed = true;

        //glm::ivec2 _mousePosition;

        game_state state_previous;
        game_state state_current;

        int m_width = 1080;
        int m_height = 720;
        bool running = true;
        bool fullscreen = false;

        //physics
        physics_simulation _physics;

        //networking
        std::unique_ptr<std::thread> _networking_thread;

        //audio
        audio_player audio;

        //networking
        network_settings net_set;
        //net_set.addrServer.Clear();
        //net_set.addrServer.ParseString("127.0.0.1");
        //net_set.addrServer.m_port = net_set.port;

        //visuals
        std::shared_ptr<Model> tempModel;
    };
}