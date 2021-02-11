
// c++ lib headers
#include <chrono>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

// other library headers
// #include <box2d/box2d.h>
#include <glm/glm.hpp>

// fightingengine headers
#include "engine/application.hpp"
#include "engine/maths_core.hpp"
#include "engine/opengl/render_command.hpp"
#include "engine/ui/profiler_panel.hpp"
using namespace fightingengine;

// game headers
#include "breakout/game.hpp"
#include "breakout/sprite_renderer.hpp"
using namespace game2d;

// Util function to log time since start of the program
void
log_time_since(const std::string& label, std::chrono::time_point<std::chrono::high_resolution_clock> start)
{
  const auto x = std::chrono::high_resolution_clock::now();
  const auto y = std::chrono::duration_cast<std::chrono::milliseconds>(x - start).count();
  std::cout << label << y << "ms" << std::endl;
}

int
main()
{
  std::cout << "booting up..." << std::endl;
  const auto app_start = std::chrono::high_resolution_clock::now();

  const int screen_width = 1366;
  const int screen_height = 768;
  Application app("2D Gaaaaaame", screen_width, screen_height);
  app.set_fps_limit(60.0f);

  //
  // Camera
  //

  glm::mat4 projection =
    glm::ortho(0.0f, static_cast<float>(screen_width), static_cast<float>(screen_height), 0.0f, -1.0f, 1.0f);

  RandomState rnd;
  Profiler profiler;

  //
  // Rendering
  //

  glm::vec4 dark_blue = glm::vec4(0.0f / 255.0f, 100.0f / 255.0f, 100.0f / 255.0f, 1.0f);
  RenderCommand::init();
  RenderCommand::set_clear_colour(dark_blue);
  RenderCommand::set_viewport(0, 0, screen_width, screen_height);
  RenderCommand::set_depth_testing(false); // disable depth testing for 2d

  Shader shader = Shader("assets/shaders/sprite.vert", "assets/shaders/sprite.frag");
  shader.bind();
  shader.set_int("image", 0);
  shader.set_mat4("projection", projection);

  //
  // TODO sound
  //

  //
  // load textures
  //
  int textures = 0;
  int tex_unit_bamboo_diffuse = textures++;
  int tex_unit_solid = textures++;
  int tex_unit_block = textures++;
  int tex_unit_paddle = textures++;
  int tex_unit_face = textures++;
  int tex_unit_kenny_nl = textures++;

  log_time_since("(Threaded) loading textures... ", app_start);
  {
    std::vector<std::pair<int, std::string>> textures_to_load;
    textures_to_load.emplace_back(tex_unit_bamboo_diffuse, "assets/textures/Bamboo/BambooWall_1K_albedo.jpg");
    textures_to_load.emplace_back(tex_unit_solid, "assets/breakout/solid_texture.png");
    textures_to_load.emplace_back(tex_unit_block, "assets/breakout/block_texture.png");
    textures_to_load.emplace_back(tex_unit_paddle, "assets/breakout/paddle.png");
    textures_to_load.emplace_back(tex_unit_face, "assets/breakout/face.png");
    textures_to_load.emplace_back(tex_unit_kenny_nl, "assets/textures/1-bit-pack-kennynl/Tilesheet/colored.png");
    // textures_to_load.emplace_back(tex_unit_kenny_nl,
    // "assets/textures/1-bit-pack-kennynl/Tilesheet/colored_packed.png");
    // textures_to_load.emplace_back(tex_unit_kenny_nl,
    // "assets/textures/1-bit-pack-kennynl/Tilesheet/monochrome.png");
    // textures_to_load.emplace_back(tex_unit_kenny_nl,
    // "assets/textures/1-bit-pack-kennynl/Tilesheet/monochrome_packed.png");

    std::vector<std::thread> threads;
    std::vector<StbLoadedTexture> loaded_textures(textures_to_load.size());

    for (int i = 0; i < textures_to_load.size(); ++i) {
      const std::pair<int, std::string>& tex_to_load = textures_to_load[i];
      threads.emplace_back([&tex_to_load, i, &loaded_textures]() {
        loaded_textures[i] = load_texture(tex_to_load.first, tex_to_load.second);
      });
    }
    for (auto& thread : threads) {
      thread.join();
    }
    for (StbLoadedTexture& l : loaded_textures) {
      bind_stb_loaded_texture(l);
    }
  }
  log_time_since("(End Threaded) textures loaded ", app_start);

  // ---- Game ----

  Breakout breakout;
  breakout.state = GameState::GAME_ACTIVE;

  // ---- box2d physics

  // b2Vec2 gravity(0.0f, 9.81f);
  // b2World world(gravity);

  // -- create a static body

  // // 1. define a body with position, damping, etc
  // b2BodyDef body_def;
  // body_def.position = { 0.0f, screen_height - 100.0f };
  // body_def.type = b2_staticBody;
  // // 2. use world obj to create body
  // b2Body* static_body = world.CreateBody(&body_def);
  // // 3. define fixtures with shape, friction, density etc
  // b2PolygonShape ground_box;
  // ground_box.SetAsBox(50.0f, 50.0f);
  // // 4. create fixtures on the body
  // static_body->CreateFixture(&ground_box, 0.0f);

  // // -- create a dynamic body
  // b2BodyDef dynamic_body_def;
  // dynamic_body_def.type = b2_dynamicBody;
  // dynamic_body_def.position.Set(0.0f, 400.0f);
  // b2Body* dynamic_body = world.CreateBody(&dynamic_body_def);
  // b2PolygonShape dynamic_box;
  // dynamic_box.SetAsBox(50.0f, 50.0f);
  // b2FixtureDef fixture_def;
  // fixture_def.shape = &dynamic_box;
  // fixture_def.density = 1.0f;
  // fixture_def.friction = 0.3f;
  // dynamic_body->CreateFixture(&fixture_def);

  // float physics_timestep = 1.0f / 60.0f;
  // int32 velocity_iterations = 6;
  // int32 position_iterations = 2;

  // levels

  breakout.levels.clear();

  GameLevel level_0;
  {
    std::vector<std::vector<int>> level_0_bricks;
    load_level_from_file(level_0_bricks, "assets/breakout/level_0.breakout");
    init_level(level_0, level_0_bricks, screen_width, static_cast<int>(screen_height / 2.0f));
  }
  breakout.levels.push_back(level_0);

  // player

  GameObject player;
  player.transform.angle = 0.0f;
  player.transform.colour = { 0.8f, 0.8f, 0.7f };
  player.transform.scale = { 100.0f, 20.0f };
  player.transform.position = { screen_width / 2.0f - player.transform.scale.x / 2.0f,
                                screen_height - player.transform.scale.y };
  player.velocity = { 500.0f, 0.0f };

  // ball

  Ball ball;
  ball.radius = 36.0f;
  ball.game_object.velocity = { 100.0f, -100.0f };
  ball.game_object.transform.scale = { ball.radius * 2.0f, ball.radius * 2.0f };
  ball.game_object.transform.position = { player.transform.position };
  ball.game_object.transform.position.x += (player.transform.scale.x / 2.0f) - ball.radius;
  ball.game_object.transform.position.y += -ball.radius * 2.0f;
  // printf("ball pos: %f %f", ball.game_object.transform.position.x, ball.game_object.transform.position.y);

  // ---- App ----

  while (app.is_running()) {

    profiler.new_frame();
    profiler.begin(Profiler::Stage::UpdateLoop);

    app.frame_begin();

    profiler.begin(Profiler::Stage::SdlInput);
    app.poll(); // input events

    float delta_time_s = app.get_delta_time();

    // Shutdown app.
    if (app.get_input().get_key_down(SDL_KeyCode::SDLK_END))
      app.shutdown();

    // Hold a key
    if (app.get_input().get_key_held(SDL_SCANCODE_F))
      printf("F is being held! \n");

    if (app.get_input().get_key_down(SDL_KeyCode::SDLK_r))
      printf("Todo - reset game");

    // if (app.get_input().get_key_down(SDL_KeyCode::SDLK_t))
    //   dynamic_body->SetTransform(b2Vec2(0.0f, 0.0f), dynamic_body->GetAngle());

    // if (app.get_input().get_mouse_lmb_held()) {
    //   glm::ivec2 mouse_pos = app.get_window().get_mouse_position();
    //   dynamic_body->SetTransform(b2Vec2(mouse_pos.x, mouse_pos.y), dynamic_body->GetAngle());
    // }

    profiler.end(Profiler::Stage::SdlInput);
    profiler.begin(Profiler::Stage::GameTick);

    if (breakout.state == GameState::GAME_ACTIVE) {
      // update user input
      update_user_input(app, delta_time_s, player, ball, screen_width);

      // update game state
      // move_ball(ball, delta_time_s, screen_width);

      // collisions
      // do_collisions_bricks( breakout.levels[0], ball);
      // do_collisions_player( player, ball );

      // physics
      // world.Step(physics_timestep, velocity_iterations, position_iterations);
    }
    profiler.end(Profiler::Stage::GameTick);
    profiler.begin(Profiler::Stage::Render);

    // rendering
    RenderCommand::clear();

    // b2Vec2 pos = dynamic_body->GetPosition();
    // float angle = dynamic_body->GetAngle();

    // draw_sprite(shader,
    //             plane,
    //             ResourceManager::get_texture("block"),
    //             { pos.x, pos.y },
    //             { 100.0f, 100.0f },
    //             angle,
    //             { 1.0f, 1.0f, 1.0f });

    // b2Vec2 static_pos = static_body->GetPosition();
    // float static_angle = static_body->GetAngle();
    // draw_sprite(shader,
    //             plane,
    //             ResourceManager::get_texture("block"),
    //             { static_pos.x, static_pos.y },
    //             { 100.0f, 100.0f },
    //             static_angle,
    //             { 1.0f, 1.0f, 1.0f });

    // draw_background ( shader, plane, background_tex, screen_width, screen_height );
    // for(auto& brick : breakout.levels[0].bricks)
    // {
    //     if(!brick.destroyed)
    //         draw_sprite( shader, plane, brick );
    // }
    // draw_sprite ( shader, plane, player );
    // draw_sprite ( shader, plane, ball.game_object );

    profiler.end(Profiler::Stage::Render);
    profiler.begin(Profiler::Stage::GuiLoop);
    //
    // GUI
    //

    app.gui_begin();

    if (ImGui::BeginMainMenuBar()) {

      if (ImGui::MenuItem("Quit", "Esc")) {
        app.shutdown();
      }

      ImGui::SameLine(ImGui::GetWindowWidth() - 154.0f);
      ImGui::Text("%.2f FPS (%.2f ms)", ImGui::GetIO().Framerate, 1000.0f / ImGui::GetIO().Framerate);

      ImGui::EndMainMenuBar();
    }

    // ImGui demo window
    // ImGui::ShowDemoWindow(&show_imgui_demo_window);

    profiler_panel::draw(profiler, delta_time_s);

    app.gui_end();

    profiler.end(Profiler::Stage::GuiLoop);
    profiler.begin(Profiler::Stage::FrameEnd);

    // end frame
    app.frame_end(delta_time_s);

    profiler.end(Profiler::Stage::FrameEnd);
    profiler.end(Profiler::Stage::UpdateLoop);
  }
}