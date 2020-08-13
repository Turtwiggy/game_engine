#include "game.h"

//Import engine stuff
#include "3d/game_object_3d.hpp"
#include "graphics/render_command.h"
#include "graphics/opengl/renderers/renderer_ray_traced.h"
#include "gui.hpp"
#include "_todo/terrain/terrain.hpp"
using namespace fightingengine;

#include <GL/glew.h>
#include <entt/entt.hpp>

#include <cstdint>
#include <string>


bool Game::process_events(Renderer& r, GameWindow& g_window, Gui& gui, Camera& camera, InputManager& input_manager)
{
    SDL_Event e;
    while (SDL_PollEvent(&e))
    {
        // If gui captures this input, do not propagate
        if (!gui.ProcessEventSdl2(e, r.get_imgui_context()))
        {
            //Update camera when mouse is grabbed
            if (g_window.IsInputGrabbed())
                camera.process_events(e);

            //Other window events
            if (e.type == SDL_QUIT)
                return false;
            else if (e.type == SDL_WINDOWEVENT
                && e.window.event == SDL_WINDOWEVENT_CLOSE
                && e.window.windowID == SDL_GetWindowID(g_window.GetHandle()))
                return false;

            switch (e.type)
            {
            case SDL_KEYDOWN:
                //keyboard specific! (need to rework for controllers)
                {
                    SDL_KeyboardEvent key_event = e.key;
                    auto key = key_event.keysym.sym;
                    input_manager.add_button_down(key);
                }
                return true;

            case SDL_KEYUP:
                //keyboard specific! (need to rework for controllers)
                {
                    SDL_KeyboardEvent key_event = e.key;
                    auto key = key_event.keysym.sym;
                    input_manager.add_button_up(key);
                }
                return true;

            case SDL_MOUSEBUTTONDOWN:
                //mouse specific
                {
                    input_manager.add_mouse_down(e.button);
                }
                return true;

            case SDL_MOUSEBUTTONUP:
                //mouse specific
                {
                    input_manager.add_mouse_up(e.button);
                }
                return true;

            }
        }
    }
    return true;
}

void Game::tick(float delta_time_in_seconds, GameState& state, float timer, InputManager& input_manager, Camera& camera)
{
    //printf("ticking state, delta_time: %f \n", delta_time_in_seconds);

    //std::shared_ptr<FGObject> cube0 = state.cubes[0];
    //cube0->transform.Position.x = 5.0f + glm::sin(timer);
    //cube0->transform.Position.y = 0.0f;
    //cube0->transform.Position.z = 5.0f + glm::cos(timer);

    //const float pi = 3.14;
    //const float frequency = 0.3f; // Frequency in Hz
    //float bouncy_val = 0.5 * (1.0 + sin(2.0 * pi * frequency * timer));

    //cube0->transform.Scale.x = glm::max(0.3f, bouncy_val);
    //cube0->transform.Scale.y = 1.0f;
    //cube0->transform.Scale.z = glm::max(0.3f, bouncy_val);
    ////printf("cube pos: %f %f %f", state.cube_pos.x, state.cube_pos.y, state.cube_pos.z);
    ////printf("lerp sin_val: %f x: %f z: %f \n ", bouncy_val);

    ////Player Cube
    //std::shared_ptr<FGObject> player_cube = state.player;
    //if (input_manager.get_key_held(SDLK_UP))
    //{
    //    player_cube->transform.Position.z += 1.0f * delta_time_in_seconds;
    //}
    //if (input_manager.get_key_held(SDLK_DOWN))
    //{
    //    player_cube->transform.Position.z -= 1.0f * delta_time_in_seconds;
    //}
    //if (input_manager.get_key_held(SDLK_LEFT))
    //{
    //    player_cube->transform.Position.x -= 1.0f * delta_time_in_seconds;
    //}
    //if (input_manager.get_key_held(SDLK_RIGHT))
    //{
    //    player_cube->transform.Position.x += 1.0f * delta_time_in_seconds;
    //}

    //Camera
    //------
    camera.process_users_input(input_manager);
    camera.update(delta_time_in_seconds);
}

void Game::fixed_tick(float fixed_delta_time_in_seconds)
{
    //printf("fixed tick");
    //advance_physics(state, fixed_delta_time);
}

void Game::render(Profiler& profiler,GameState& state,Renderer& rend,Camera& camera,Gui& gui,GameWindow& window, float exposure )
{
    //Begin Frame
    {
        profiler.Begin(Profiler::Stage::NewFrame);
        rend.new_frame(window.GetHandle());
        profiler.End(Profiler::Stage::NewFrame);
    }

    //Main rendering
    {
        profiler.Begin(Profiler::Stage::MainDraw);

        fightinggame::draw_scene_desc drawDesc
        (
            camera,
            window,
            exposure
        );

        //update gamestate timer
        state.time = timer;

        rend.renderer_impl->draw_pass
        (
            drawDesc,
            state
        );
        profiler.End(Profiler::Stage::MainDraw);
    }

    //Render GUI
    {
        profiler.Begin(Profiler::Stage::GuiLoop);
        if (gui.Loop(*this, profiler))
        {
            running = false;
            return;
        }
        profiler.End(Profiler::Stage::GuiLoop);
    }

    //End Frame
    {
        profiler.Begin(Profiler::Stage::EndFrame);
        rend.end_frame(window.GetHandle());
        profiler.End(Profiler::Stage::EndFrame);
    }
}
