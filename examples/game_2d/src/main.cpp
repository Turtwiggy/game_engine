
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
  // TODO sound
  //

  //
  // load textures
  //
  int NUM_TEXTURES = 0;
  const int tex_unit_bamboo_diffuse = NUM_TEXTURES++;
  const int tex_unit_solid = NUM_TEXTURES++;
  const int tex_unit_block = NUM_TEXTURES++;
  const int tex_unit_paddle = NUM_TEXTURES++;
  const int tex_unit_face = NUM_TEXTURES++;
  const int tex_unit_kenny_nl = NUM_TEXTURES;
  std::cout << "planning to load " << NUM_TEXTURES << " textures" << std::endl;

  log_time_since("(Threaded) loading textures... ", app_start);
  {
    std::vector<std::pair<int, std::string>> textures_to_load;
    textures_to_load.emplace_back(tex_unit_bamboo_diffuse, "assets/textures/Bamboo/BambooWall_1K_albedo.jpg");
    textures_to_load.emplace_back(tex_unit_solid, "assets/breakout/solid_texture.png");
    textures_to_load.emplace_back(tex_unit_block, "assets/breakout/block_texture.png");
    textures_to_load.emplace_back(tex_unit_paddle, "assets/breakout/paddle.png");
    textures_to_load.emplace_back(tex_unit_face, "assets/textures/octopus.png");
    textures_to_load.emplace_back(tex_unit_kenny_nl, "assets/textures/kennynl_1bit_pack/Tilesheet/colored.png");

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

  //
  // Rendering
  //

  glm::vec4 dark_blue = glm::vec4(0.0f / 255.0f, 100.0f / 255.0f, 100.0f / 255.0f, 1.0f);
  RenderCommand::init();
  RenderCommand::set_clear_colour(dark_blue);
  RenderCommand::set_viewport(0, 0, screen_width, screen_height);
  RenderCommand::set_depth_testing(false); // disable depth testing for 2d

  Shader shader = Shader("2d/sprite.vert", "2d/sprite.frag");
  shader.bind();
  shader.set_mat4("projection", projection);
  shader.set_int("tex", tex_unit_kenny_nl);

  // player

  GameObject player;
  player.transform.angle = 0.0f;
  player.transform.colour = { 1.0f, 1.0f, 1.0f, 1.0f };
  player.transform.scale = { screen_width, screen_height };
  player.transform.position = { 0.0f, 0.0f };
  player.tex_slot = tex_unit_kenny_nl;
  player.velocity = { 500.0f, 500.0f };

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

  // breakout.levels.clear();
  // GameLevel level_0;
  // {
  //   std::vector<std::vector<int>> level_0_bricks;
  //   load_level_from_file(level_0_bricks, "assets/breakout/level_0.breakout");
  //   init_level(level_0, level_0_bricks, screen_width, static_cast<int>(screen_height / 2.0f));
  // }
  // breakout.levels.push_back(level_0);

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

    if (app.get_input().get_key_down(SDL_KeyCode::SDLK_ESCAPE))
      app.shutdown();

    // Shader hot reload
    if (app.get_input().get_key_down(SDL_KeyCode::SDLK_r))
      reload_shader_program(&shader.ID, "2d/sprite.vert", "2d/sprite.frag");

    // if (app.get_input().get_key_down(SDL_KeyCode::SDLK_t))
    //   dynamic_body->SetTransform(b2Vec2(0.0f, 0.0f), dynamic_body->GetAngle());

    // if (app.get_input().get_mouse_lmb_held()) {
    //   glm::ivec2 mouse_pos = app.get_window().get_mouse_position();
    //   dynamic_body->SetTransform(b2Vec2(mouse_pos.x, mouse_pos.y), dynamic_body->GetAngle());
    // }

    profiler.end(Profiler::Stage::SdlInput);
    profiler.begin(Profiler::Stage::GameTick);

    if (breakout.state == GameState::GAME_ACTIVE) {

      float velocity_x = player.velocity.x * delta_time_s;
      float velocity_y = player.velocity.y * delta_time_s;

      if (app.get_input().get_key_held(SDL_SCANCODE_A))
        player.transform.position.x -= velocity_x;

      if (app.get_input().get_key_held(SDL_SCANCODE_D))
        player.transform.position.x += velocity_x;

      if (app.get_input().get_key_held(SDL_SCANCODE_W))
        player.transform.position.y -= velocity_y;

      if (app.get_input().get_key_held(SDL_SCANCODE_S))
        player.transform.position.y += velocity_y;
    }

    // update game state
    // move_ball(ball, delta_time_s, screen_width);

    // collisions
    // do_collisions_bricks( breakout.levels[0], ball);
    // do_collisions_player( player, ball );

    // physics
    // world.Step(physics_timestep, velocity_iterations, position_iterations);
    profiler.end(Profiler::Stage::GameTick);
    profiler.begin(Profiler::Stage::Render);

    // rendering
    RenderCommand::set_clear_colour(dark_blue);
    RenderCommand::clear();

    // b2Vec2 pos = dynamic_body->GetPosition();
    // float angle = dynamic_body->GetAngle();

    sprite_renderer::draw_sprite(shader, player);

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

    ImGui::Begin("Game Info");
    ImGui::Text("%f %f", player.transform.position.x, player.transform.position.y);
    ImGui::End();

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