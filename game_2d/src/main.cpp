#pragma once

#include "engine/core/application.hpp"
#include "engine/renderer/render_command.hpp"
#include "engine/renderer/shader.hpp"
#include "engine/resources/resource_manager.hpp"
#include "engine/2d/camera_2d.hpp"

using namespace fightingengine;

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <SDL2/SDL.h>
#undef main
//^thanks SDL2

//Resources
//https://learnopengl.com/In-Practice/2D-Game/Levels

enum BreakoutGameState {
    GAME_ACTIVE,
    GAME_MENU,
    GAME_WIN
};

struct BreakoutGameObject {
    glm::vec2   Position, Size, Velocity;
    glm::vec3   Colour;
    float       Rotation;
    bool        Destroyed;

    Texture2D Sprite;

    BreakoutGameObject()
        : Position(0.0f, 0.0f), Size(1.0f, 1.0f), Velocity(0.0f)
        , Colour(1.0f), Rotation(0.0f), Sprite(), Destroyed(false) { }

    BreakoutGameObject(
        glm::vec2 pos, glm::vec2 size, Texture2D sprite, glm::vec3 color = glm::vec3(1.0f),
        glm::vec2 velocity = glm::vec2(0.0f, 0.0f))
        : Position(pos), Size(size), Velocity(velocity)
        , Colour(color), Rotation(0.0f), Sprite(sprite), Destroyed(false) { }
    ;

    void Draw(SpriteRenderer& renderer)
    {
        renderer.draw_sprite(this->Sprite, this->Position, this->Size, this->Rotation, this->Colour);
    }
};

struct BreakoutGameLevel {
    // level state
    std::vector<BreakoutGameObject> Bricks;

    BreakoutGameLevel() { }

    void Load(const char* file, unsigned int levelWidth, unsigned int levelHeight)
    {

    }

    void Draw(SpriteRenderer& renderer)
    {

    }

    bool IsCompleted()
    {

    }

private:
    // initialize level from tile data
    void init(std::vector<std::vector<unsigned int>> tileData,
        unsigned int levelWidth, unsigned int levelHeight)
    {

    }

};

struct Breakout {

    SpriteRenderer* Renderer;

    std::vector<BreakoutGameLevel>  levels;
    unsigned int                    level;
    BreakoutGameState               state;

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

        Renderer->draw_sprite(t, pos, scale, angle, colour);
    }
};

int main(int argc, char** argv)
{
    int width = 1080;
    int height = 720;
    Application app("Breakout", width, height);
    app.set_fps_limit(144.0);

    int pixel_sheet_size = 16;
    Camera2D camera(pixel_sheet_size);

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


