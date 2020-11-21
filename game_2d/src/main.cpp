
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
    const int screen_width  = 1366;
    const int screen_height = 768;
    Application app("Breakout", screen_width, screen_height);
    app.set_fps_limit( 60.0f );   
    //app.remove_fps_limit();

    // ---- Rendering & Textures ----

    glm::vec4 dark_blue = glm::vec4(0.0f/255.0f, 100.0f/255.0f, 100.0f/255.0f, 1.0f);
    RenderCommand::init();
    RenderCommand::set_clear_colour(dark_blue);
    RenderCommand::set_viewport(0, 0, screen_width, screen_height);
    RenderCommand::set_depth_testing(false); //disable depth testing for 2d

    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(screen_width), static_cast<float>(screen_height), 0.0f, -1.0f, 1.0f);
    Shader shader = ResourceManager::load_shader( "assets/shaders/2d", {"sprite.vert", "sprite.frag"}, "sprite_texture" );
    shader.bind();
    shader.set_int("image", 0);
    shader.set_mat4("projection", projection);
    ResourceManager::load_texture( "assets/textures/Bamboo/BambooWall_1K_albedo.jpg", "background" );
    ResourceManager::load_texture( "assets/breakout/solid_texture.png", "block_solid" );
    ResourceManager::load_texture( "assets/breakout/block_texture.png", "block" );
    ResourceManager::load_texture( "assets/breakout/paddle.png",        "paddle" );
    fightingengine::primitives::Plane plane { 1, 1 };

    // ---- Game ----

    // state
    
    Breakout breakout;
    breakout.state = GameState::GAME_ACTIVE;
    init_breakout_levels(breakout.levels, screen_width, screen_height);

    // player

    glm::vec2 player_size { 100.0f, 20.0f };
    GameObject player;
    player.transform.angle = 0.0f;
    player.transform.colour = { 1.0f, 0.0f, 0.0f };
    player.transform.position = 
    { 
        screen_width / 2.0f - player_size.x / 2.0f, 
        screen_height - player_size.y 
    };
    player.transform.scale = player_size;
    player.texture = ResourceManager::get_texture("paddle");
    player.velocity = { 500.0f, 0.0f };

    // ball

    //TODO(Turtwiggy)

    // ---- App ----

    while(app.is_running())
    {
        app.frame_begin();
        app.poll(); //input events

        float delta_time_s = app.get_delta_time();

        // Shutdown app
        if (app.get_input().get_key_down(SDL_KeyCode::SDLK_END))
            app.shutdown();

        // Hold a key
        if (app.get_input().get_key_held(SDL_SCANCODE_F))
            printf("F is being held! \n");

        //update_user_input();
        if(breakout.state == GameState::GAME_ACTIVE)
        {
            if(app.get_input().get_key_held(SDL_SCANCODE_A))
                player.transform.position.x -= player.velocity.x * delta_time_s;
            if(app.get_input().get_key_held(SDL_SCANCODE_D))
                player.transform.position.x += player.velocity.x * delta_time_s;
        }

        update_game_state();

        update_renderer(shader, plane, screen_width, screen_height, breakout, player);
        
        // ImGUI
        app.gui_begin();

            // ImGui demo window
            //ImGui::ShowDemoWindow(&show_imgui_demo_window);

            // Show FPS
            ImGui::Begin("FPS");
            ImGui::Text("FPS %f", app.get_raw_fps(delta_time_s));
            ImGui::Text("Average App FPS %f", app.get_average_fps());
            ImGui::End();

        app.gui_end();

        // end frame
        app.frame_end(delta_time_s);
    }
}