
//c++ lib headers
#include <iostream>

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
    init_breakout(game_data, width, height);
    std::cout << "size of gamedata: " << sizeof(game_data) << " bytes" << std::endl;

    //The sprite renderer
    SpriteRenderer renderer;

    while(app.is_running())
    {
        app.frame_begin();

        //key input events
        app.poll(); 

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
        renderer.draw_sprite(game_data.sprite_texture, )

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
