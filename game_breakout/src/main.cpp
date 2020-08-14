#pragma once

#include "engine/core/application.h"
#include "engine/renderer/render_command.h"
using namespace fightingengine;

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <SDL2/SDL.h>

#undef main
//^thanks SDL2

struct BreakoutGameState {

    enum State {
        GAME_ACTIVE,
        GAME_MENU,
        GAME_WIN
    };

    void update(float delta_time)
    {

    }

};

struct Breakout {

    BreakoutGameState state;

    void tick(float delta_time)
    {
        state.update(delta_time);
    }

};


int main(int argc, char** argv)
{
    int width = 1080;
    int height = 720;
    Application app("Breakout", width, height);

    Breakout game;

    //Renderer (TODO move to renderer)
    RenderCommand::init();
    RenderCommand::set_viewport(0, 0, width, height);

    while (app.is_running())
    {
        app.frame_begin();
        app.poll();
        float delta_time_s = app.get_delta_time();

        if (app.get_input().get_key_down(SDL_KeyCode::SDLK_ESCAPE))
            app.shutdown();

        if (app.get_input().get_key_down(SDL_KeyCode::SDLK_m))
            app.get_window().ToggleMouseCaptured();

        game.tick(delta_time_s);

        //rendering
        RenderCommand::set_clear_colour(glm::vec4(0.2f, 0.6f, 0.2f, 1.0f));
        RenderCommand::clear();

        app.gui_begin();

        ImGui::Begin("Game Stats");
        ImGui::Text("FPS %f", app.get_average_fps() );
        ImGui::End();

        app.gui_end();

        app.frame_end(delta_time_s);
    }

    return 0;
}


