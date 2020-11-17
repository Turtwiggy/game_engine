
//c++ lib headers
#include <iostream>
#include <stdio.h>

//other library headers
#include <glm/glm.hpp>

//your project headers
#include "engine/2d/renderer/sprite_renderer.hpp"
#include "engine/core/application.hpp"
#include "engine/graphics/render_command.hpp"
#include "engine/resources/resource_manager.hpp"
using namespace fightingengine;

//gamestuff
#include "breakout/game.hpp"
using namespace game2d;

int main()
{
    std::cout << "Hello world!" << std::endl;

    uint32_t width = 1366;
    uint32_t height = 768;
    Application app("Breakout", width, height);
    app.set_fps_limit(60.0f);   
    //app.remove_fps_limit();

    //The game!
    BreakoutData game_data;
    std::cout << "size of gamedata: " << sizeof(game_data) << " bytes" << std::endl;

    RenderCommand::init();
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, -1.0f, 1.0f);
    Shader shader = ResourceManager::load_shader( "assets/shaders/2d/", {"sprite.vert", "sprite.frag"}, "sprite_texture" );
    Texture2D tex =  ResourceManager::load_texture("assets/textures/octopus.png", "octopus");
    shader.bind();
    shader.set_int("image", 0);
    shader.set_mat4("projection", projection);
    fightingengine::primitives::Plane plane {1, 1};

    Transform t;
    t.position = {200.0f, 200.0f};
    t.scale = {720.0f, 720.0f*(9.0f/16.0f)};
    t.angle = 180.0f;
    t.colour = {0.0f, 1.0f, 0.0f};

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
        //draw_sprite(renderer_data, t);
        draw_sprite(
            shader,
            plane.vao,
            tex,
            t.position,
            t.scale,
            t.angle,
            t.colour
        );
        draw_sprite(
            shader,
            plane.vao,
            tex,
            {500.0f, 500.0f},
            t.scale,
            t.angle,
            t.colour
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

//struct Breakout {
//
//    SpriteRenderer* Renderer;
//
//    std::vector<BreakoutGameLevel>  levels;
//    unsigned int                    level;
//    BreakoutGameState               state;
//
//    void init(int width, int height)
//    {
//        std::vector<std::string> files = { "example.frag", "example.vert" };
//        Shader s = ResourceManager::load_shader("assets/", files, "example");
//
//        // configure shaders
//        glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, -1.0f, 1.0f);
//
//        s.bind();
//        s.setInt("image", 0);
//        s.setMat4("projection", projection);
//
//        // set render-specific controls
//        Renderer = new SpriteRenderer(s);
//
//        // load textures
//        ResourceManager::load_texture("assets/textures/marble.jpg", true, "marble");
//    }
//
//    void tick(float delta_time)
//    {
//
//    }
//
//    void render()
//    {
//        Texture2D t = ResourceManager::get_texture("marble");
//
//        glm::vec2 pos(200.0f, 200.0f);
//        glm::vec2 scale(300.0f, 300.0f);
//        glm::vec3 colour(1.0f, 0.0f, 0.0f);
//        float angle = 45.0f;
//
//        Renderer->draw_sprite(t, pos, scale, angle, colour);
//    }
//};
