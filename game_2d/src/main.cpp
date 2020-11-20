
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
    app.set_fps_limit(60.0f);   
    //app.remove_fps_limit();

    // ---- Rendering & Textures ----

    RenderCommand::init();
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(screen_width), static_cast<float>(screen_height), 0.0f, -1.0f, 1.0f);
    Shader shader = ResourceManager::load_shader( "assets/shaders/2d", {"sprite.vert", "sprite.frag"}, "sprite_texture" );
    ResourceManager::load_texture( "assets/breakout/solid_texture.png", "block_solid" );
    ResourceManager::load_texture( "assets/breakout/block_texture.png", "block" );
    ResourceManager::load_texture( "assets/textures/Bamboo/BambooWall_1K_albedo.jpg", "background" );
    shader.bind();
    shader.set_int("image", 0);
    shader.set_mat4("projection", projection);
    fightingengine::primitives::Plane plane {1, 1};

    // ---- Game ----

    //state
    
    Breakout game_state;
    game_state.state = GameState::GAME_ACTIVE;

    //levels

    std::vector<std::vector<int>> level_0_bricks;
    load_level_from_file(level_0_bricks, "assets/breakout/level_0.breakout");
    GameLevel level_0;
    init_level(level_0, level_0_bricks, screen_width, static_cast<int>(screen_height / 2.0f));    

    std::vector<std::vector<int>> level_1_bricks;
    load_level_from_file(level_1_bricks, "assets/breakout/level_1.breakout");
    GameLevel level_1;
    init_level(level_1, level_1_bricks, screen_width, static_cast<int>(screen_height / 2.0f));  

    std::vector<std::vector<int>> level_2_bricks;
    load_level_from_file(level_2_bricks, "assets/breakout/level_2.breakout");
    GameLevel level_2;
    init_level(level_2, level_2_bricks, screen_width, static_cast<int>(screen_height / 2.0f));  

    std::vector<std::vector<int>> level_3_bricks;
    load_level_from_file(level_3_bricks, "assets/breakout/level_3.breakout");
    GameLevel level_3;
    init_level(level_3, level_3_bricks, screen_width, static_cast<int>(screen_height / 2.0f));  

    //player

    glm::vec2 player_size { 100.0f, 20.0f };
    float player_velocity = 500.0f;

    // ---- App ----

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
        if(game_state.state == GameState::GAME_ACTIVE)
        {   
            for(auto& brick : level_0.bricks)
            {
                if(!brick.destroyed)
                    draw_sprite( shader, plane, brick );
            }
        }

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