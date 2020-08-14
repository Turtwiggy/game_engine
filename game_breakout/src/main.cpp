#pragma once

#include "engine/core/application.h"
#include "engine/renderer/render_command.h"
#include "engine/renderer/shader.hpp"
#include "engine/resources/resource_manager.h"
using namespace fightingengine;

#include "sprite_renderer.h"

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <SDL2/SDL.h>
#undef main
//^thanks SDL2

enum BreakoutGameState {
    GAME_ACTIVE,
    GAME_MENU,
    GAME_WIN
};

struct Breakout {

    SpriteRenderer* Renderer;
    BreakoutGameState state;

    void init(int width, int height)
    {
        std::vector<std::string> files = { "example.frag", "example.vert" };
        Shader s = ResourceManager::load_shader("assets/", files, "example");

        // configure shaders
        glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, -1.0f, 1.0f);

        s.bind();
        s.setInt("image", 0);
        s.setMat4("projection", projection);

        // set render-specific controls
        Renderer = new SpriteRenderer(s);

        // load textures
        ResourceManager::load_texture("assets/textures/marble.jpg", true, "marble");
    }

    void tick(float delta_time)
    {

    }

    void render()
    {
        Texture2D t = ResourceManager::get_texture("marble");

        glm::vec2 pos(200.0f, 200.0f);
        glm::vec2 scale(300.0f, 300.0f);
        glm::vec3 colour(1.0f, 0.0f, 0.0f);
        float angle = 45.0f;

        Renderer->DrawSprite(t, pos, scale, angle, colour);
    }
};

int main(int argc, char** argv)
{
    int width = 1080;
    int height = 720;
    Application app("Breakout", width, height);
    app.set_fps_limit(144.0);

    Breakout game;
    game.init(width, height);

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

        //if (app.get_event().window_unfocus())
        //    game.pause();

        game.tick(delta_time_s);

        //rendering
        RenderCommand::set_clear_colour(glm::vec4(0.2f, 0.6f, 0.2f, 1.0f));
        RenderCommand::clear();
        game.render();

        app.gui_begin();

        ImGui::Begin("Game Stats");
        ImGui::Text("FPS %f", app.get_average_fps());
        ImGui::End();

        app.gui_end();

        app.frame_end(delta_time_s);
    }

    return 0;
}


