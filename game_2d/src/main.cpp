
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
    ResourceManager::load_texture( "assets/breakout/face.png",          "ball" );
    fightingengine::primitives::Plane plane { 1, 1 };

    Texture2D* background_tex = ResourceManager::get_texture("background");

    // ---- Game ----
    
    Breakout breakout;
    breakout.state = GameState::GAME_ACTIVE;

    // levels

    breakout.levels.clear();

    std::vector<std::vector<int>> level_0_bricks;
    load_level_from_file(level_0_bricks, "assets/breakout/level_0.breakout");
    GameLevel level_0;
    init_level(level_0, level_0_bricks, screen_width, static_cast<int>(screen_height / 2.0f));  
    breakout.levels.push_back(level_0);

    // player

    GameObject player { ResourceManager::get_texture("paddle") };
    player.transform.angle = 0.0f;
    player.transform.colour = { 0.8f, 0.8f, 0.7f };
    player.transform.scale = { 100.0f, 20.0f };
    player.transform.position = 
    { 
        screen_width / 2.0f - player.transform.scale.x / 2.0f, 
        screen_height - player.transform.scale.y
    };    
    player.velocity = { 500.0f, 0.0f };

    // ball

    Ball ball { ResourceManager::get_texture( "ball" ) };
    ball.radius = 36.0f;
    ball.game_object.velocity = { 100.0f, -300.0f };
    ball.game_object.transform.scale = { ball.radius * 2.0f, ball.radius * 2.0f };
    ball.game_object.transform.position = { player.transform.position };
    ball.game_object.transform.position.x += (player.transform.scale.x / 2.0f) - ball.radius;
    ball.game_object.transform.position.y += -ball.radius * 2.0f;
    //printf("ball pos: %f %f", ball.game_object.transform.position.x, ball.game_object.transform.position.y);

    // ---- App ----

    while(app.is_running())
    {
        app.frame_begin();
        app.poll(); //input events

        float delta_time_s = app.get_delta_time();

        // Shutdown app
        if ( app.get_input().get_key_down( SDL_KeyCode::SDLK_END ) )
            app.shutdown();

        // Hold a key
        if ( app.get_input().get_key_held(SDL_SCANCODE_F) )
            printf("F is being held! \n");

        if ( app.get_input().get_key_down( SDL_KeyCode::SDLK_r ) )
            printf("Todo - reset game");

        if ( breakout.state == GameState::GAME_ACTIVE )
        {
            // update user input
            update_user_input( app, delta_time_s, player, ball, screen_width );

            // update game state
            move_ball( ball, delta_time_s, screen_width );

            // collisions
            do_collisions_bricks( breakout.levels[0], ball);
            do_collisions_player( player, ball );

            // rendering
            RenderCommand::clear();
            draw_background ( shader, plane, background_tex, screen_width, screen_height );
            for(auto& brick : breakout.levels[0].bricks)
            {
                if(!brick.destroyed)
                    draw_sprite( shader, plane, brick );
            }
            draw_sprite ( shader, plane, player );        
            draw_sprite ( shader, plane, ball.game_object );     
        }   

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