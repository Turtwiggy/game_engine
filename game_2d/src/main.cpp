
//c++ lib headers
#include <iostream>
#include <stdio.h>

//other library headers
#include <glm/glm.hpp>

//fightingengine headers
#include "engine/core/application.hpp"
#include "engine/graphics/render_command.hpp"
#include "engine/resources/resource_manager.hpp"
using namespace fightingengine;

//game headers
#include "breakout/game.hpp"
#include "breakout/sprite_renderer.hpp"
using namespace game2d;

int main()
{
    uint32_t width = 1366;
    uint32_t height = 768;
    Application app("Breakout", width, height);
    app.set_fps_limit(60.0f);   
    //app.remove_fps_limit();

    //The game!
    Breakout game_state;
    std::cout << "size of gamedata: " << sizeof(game_state) << " bytes" << std::endl;

    RenderCommand::init();
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, -1.0f, 1.0f);
    Shader shader = ResourceManager::load_shader( "assets/shaders/2d", {"sprite.vert", "sprite.frag"}, "sprite_texture" );
    Texture2D tex = ResourceManager::load_texture( "assets/textures/octopus.png", "octopus");
    Texture2D tex2 = ResourceManager::load_texture( "assets/textures/grass.png", "grass");
    shader.bind();
    shader.set_int("image", 0);
    shader.set_mat4("projection", projection);
    fightingengine::primitives::Plane plane {1, 1};

    GameObject go;
    {
        Transform t;
        t.position = {200.0f, 200.0f};
        t.scale = {400.0f, 400.0f*(9.0f/16.0f)};
        t.angle = 0.0f;
        t.colour = {0.0f, 1.0f, 0.0f};
        go.texture = tex2;
        go.transform = t;
    }

    GameLevel level;
    load_game_level(level, "assets/breakout/level_0.breakout", 10, 10);

    while(app.is_running())
    {
        app.frame_begin();
        app.poll(); //input events

        float delta_time_s = app.get_delta_time();

        //Shutdown app
        if (app.get_input().get_key_down(SDL_KeyCode::SDLK_END))
            app.shutdown();

        //Hold a key
        if (app.get_input().get_key_held(SDL_SCANCODE_F))
            printf("F is being held! \n");

        //Example OpenGL usage
        glm::vec4 dark_blue = glm::vec4(0.0f/255.0f, 100.0f/255.0f, 100.0f/255.0f, 1.0f);
        RenderCommand::set_clear_colour(dark_blue);
        RenderCommand::clear();

        //render
        draw_sprite( shader, plane, go );
        draw_sprite( shader, plane, tex,
            {500.0f, 500.0f},   //pos
            {200.0f, 200.0f},   //scale
            30.0f,              //angle
            {1.0f, 0.3f, 0.3f}  //colour
        );

        //ImGUI
        app.gui_begin();

            //ImGui demo window
            //ImGui::ShowDemoWindow(&show_imgui_demo_window);

            //Show FPS
            ImGui::Begin("FPS");
            ImGui::Text("FPS %f", app.get_raw_fps(delta_time_s));
            ImGui::Text("Average App FPS %f", app.get_average_fps());
            ImGui::End();

        app.gui_end();

        //end frame
        app.frame_end(delta_time_s);
    }

}