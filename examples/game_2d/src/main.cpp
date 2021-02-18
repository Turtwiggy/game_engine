
// c++ lib headers
#include <chrono>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

// other library headers
#include <box2d/box2d.h>
#include <glm/glm.hpp>

// fightingengine headers
#include "engine/application.hpp"
#include "engine/maths_core.hpp"
#include "engine/opengl/render_command.hpp"
#include "engine/ui/profiler_panel.hpp"
using namespace fightingengine;

// game headers
#include "game.hpp"
#include "sprite_renderer.hpp"
using namespace game2d;

// Util function to log time since start of the program
void
log_time_since(const std::string& label, std::chrono::time_point<std::chrono::high_resolution_clock> start)
{
  const auto x = std::chrono::high_resolution_clock::now();
  const auto y = std::chrono::duration_cast<std::chrono::milliseconds>(x - start).count();
  std::cout << label << y << "ms" << std::endl;
}

struct sprite_on_spritesheet
{
  int x = 0;
  int y = 0;

  sprite_on_spritesheet(int x, int y)
    : x(x)
    , y(y)
  {}
};

// other things to consider
// colisions
// particles
// post processing
// rendering text

int
main()
{
  std::cout << "booting up..." << std::endl;
  const auto app_start = std::chrono::high_resolution_clock::now();

  const int screen_width = 1366;
  const int screen_height = 768;
  Application app("2D Game", screen_width, screen_height);
  app.set_fps_limit(60.0f);

  float screen_width_f = static_cast<float>(screen_width);
  float screen_height_f = static_cast<float>(screen_height);
  glm::mat4 projection = glm::ortho(0.0f, screen_width_f, screen_height_f, 0.0f, -1.0f, 1.0f);

  RandomState rnd;
  Profiler profiler;

  //
  // TODO sound
  //

  //
  // load textures
  //
  const int tex_unit_kenny_nl = 0;

  log_time_since("(Threaded) loading textures... ", app_start);
  {
    std::vector<std::pair<int, std::string>> textures_to_load;
    textures_to_load.emplace_back(tex_unit_kenny_nl,
                                  "assets/textures/kennynl_1bit_pack/Tilesheet/monochrome_transparent_packed.png");

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

  // -- create a dynamic body transform
  GameObject player;
  player.transform.angle = 90.0f;
  player.transform.colour = { 0.0f, 1.0f, 0.0f, 0.6f };
  player.transform.scale = { 100.0f, 100.0f };
  player.transform.position = { screen_width / 2.0f, screen_height / 2.0f };
  player.tex_slot = tex_unit_kenny_nl;
  player.velocity = { 2.0f, 2.0f };

  //
  // Rendering
  //
  sprite_on_spritesheet boat{ 10, 19 };
  sprite_on_spritesheet white_square{ 8, 5 };

  glm::vec4 dark_blue = glm::vec4(0.0f / 255.0f, 100.0f / 255.0f, 100.0f / 255.0f, 1.0f);
  RenderCommand::init();
  RenderCommand::set_clear_colour(dark_blue);
  RenderCommand::set_viewport(0, 0, screen_width, screen_height);
  RenderCommand::set_depth_testing(false); // disable depth testing for 2d

  Shader sprite_shader = Shader("2d_texture.vert", "2d_spritesheet.frag");
  sprite_shader.bind();
  sprite_shader.set_mat4("projection", projection);
  sprite_shader.set_int("tex", tex_unit_kenny_nl);

  Shader tex_shader = Shader("2d_texture.vert", "2d_texture.frag");
  tex_shader.bind();
  tex_shader.set_mat4("projection", projection);

  GameObject tex_obj;
  tex_obj.transform.angle = 0.0f;
  tex_obj.transform.colour = { 1.0f, 1.0f, 1.0f, 1.0f };
  tex_obj.transform.scale = { 768.0f, 352.0f };
  tex_obj.transform.position = { 0.0f, 20.0f };
  tex_obj.tex_slot = tex_unit_kenny_nl;
  tex_obj.velocity = { 0.0f, 0.0f };

  // ---- Game ----

  GameState state = GameState::GAME_ACTIVE;
  std::vector<glm::vec2> objects;

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

    //
    // Settings: Shutdown app
    //
    if (app.get_input().get_key_down(SDL_KeyCode::SDLK_ESCAPE))
      app.shutdown();

    //
    // Settings: Fullscreen
    //
    if (app.get_input().get_key_down(SDL_KeyCode::SDLK_f)) {
      app.get_window().toggle_fullscreen(); // SDL2 window toggle
      glm::ivec2 screen_size = app.get_window().get_size();
      RenderCommand::set_viewport(0, 0, screen_size.x, screen_size.y);
    }

    //
    // Settings: Pause
    //
    if (app.get_input().get_key_down(SDL_KeyCode::SDLK_p)) {
      state = state == GameState::GAME_PAUSED ? GameState::GAME_ACTIVE : GameState::GAME_PAUSED;
    }

    //
    // Shader hot reloading
    //
    if (app.get_input().get_key_down(SDL_KeyCode::SDLK_r)) {
      reload_shader_program(&sprite_shader.ID, "2d_texture.vert", "2d_spritesheet.frag");
      sprite_shader.bind();
      sprite_shader.set_mat4("projection", projection);
      sprite_shader.set_int("tex", tex_unit_kenny_nl);
      sprite_shader.set_int("desired_x", 1);
      sprite_shader.set_int("desired_x", 0);
    }

    //
    // Editor: Add object
    //
    if (app.get_input().get_mouse_lmb_down()) {
      glm::ivec2 mouse_pos = app.get_input().get_mouse_pos();
      printf("(game) left mouse clicked %i %i \n", mouse_pos.x, mouse_pos.y);
      objects.push_back(glm::vec2(mouse_pos.x, mouse_pos.y));
    }

    // if (app.get_input().get_key_down(SDL_KeyCode::SDLK_t))
    //   dynamic_body->SetTransform(b2Vec2(0.0f, 0.0f), dynamic_body->GetAngle());

    // if (app.get_input().get_mouse_lmb_held()) {
    //   glm::ivec2 mouse_pos = app.get_window().get_mouse_position();
    //   dynamic_body->SetTransform(b2Vec2(mouse_pos.x, mouse_pos.y), dynamic_body->GetAngle());
    // }

    profiler.end(Profiler::Stage::SdlInput);
    profiler.begin(Profiler::Stage::GameTick);

    float x = 0;
    float y = 0;

    if (state == GameState::GAME_ACTIVE) {

      const float speed = 10.0f;
      float velocity_x = delta_time_s * speed;
      float velocity_y = delta_time_s * speed;

      // standard lrud
      // if (app.get_input().get_key_held(SDL_SCANCODE_A))
      //   player.transform.position.x -= velocity_x;
      // if (app.get_input().get_key_held(SDL_SCANCODE_D))
      //   player.transform.position.x += velocity_x;
      // if (app.get_input().get_key_held(SDL_SCANCODE_W))
      //   player.transform.position.y -= velocity_y;
      // if (app.get_input().get_key_held(SDL_SCANCODE_S))
      //   player.transform.position.y += velocity_y;

      // spaceship lrud
      float angle_speed = 100.0f;

      if (app.get_input().get_key_held(SDL_SCANCODE_W)) {
        player.velocity.y += velocity_y;
      }
      if (app.get_input().get_key_held(SDL_SCANCODE_S)) {
        player.velocity.y -= velocity_y;
      }

      float turn_velocity_x = player.velocity.y; // same as y so turning doesnt feel weird
      float turn_velocity_y = player.velocity.y;

      // if (app.get_input().get_key_held(SDL_SCANCODE_SPACE))
      //   turn_velocity_x /= 2.0f;
      if (app.get_input().get_key_held(SDL_SCANCODE_SPACE)) {
        // turn_velocity_x = 0.0f;
        turn_velocity_y *= 0.1f;
        turn_velocity_x *= 0.1f;

        // increase turn speed
        angle_speed *= 5.0f;
      }

      if (app.get_input().get_key_held(SDL_SCANCODE_D))
        player.transform.angle += delta_time_s * angle_speed;
      if (app.get_input().get_key_held(SDL_SCANCODE_A))
        player.transform.angle -= delta_time_s * angle_speed;

      // if (app.get_input().get_key_held(SDL_SCANCODE_LEFT))
      //   player.velocity.x -= velocity_x;
      // if (app.get_input().get_key_held(SDL_SCANCODE_RIGHT))
      //   player.velocity.x += velocity_x;

      // update get vector based on angle
      x = glm::sin(glm::radians(player.transform.angle)) * turn_velocity_x;
      y = -glm::cos(glm::radians(player.transform.angle)) * turn_velocity_y;
      // float y = -glm::cos(glm::radians(player.transform.angle)) * speed; <!-- accelerate more in one axis

      player.transform.position.x += x;
      player.transform.position.y += y;
    }

    profiler.end(Profiler::Stage::GameTick);
    profiler.begin(Profiler::Stage::Render);
    //
    // rendering
    //
    RenderCommand::set_clear_colour(dark_blue);
    RenderCommand::clear();

    sprite_shader.bind();
    sprite_shader.set_int("desired_x", white_square.x);
    sprite_shader.set_int("desired_y", white_square.y);
    sprite_renderer::draw_sprite(sprite_shader, player);

    sprite_shader.set_int("desired_x", boat.x);
    sprite_shader.set_int("desired_y", boat.y);

    for (auto& object : objects) {
      glm::vec2 size = glm::vec2(50.0f, 50.0f);
      glm::vec3 colour = glm::vec3(1.0f, 0.0f, 0.0f);
      sprite_renderer::draw_sprite(sprite_shader, object, size, 0.0f, colour);
    }

    tex_shader.bind();
    tex_shader.set_int("tex", tex_unit_kenny_nl);
    sprite_renderer::draw_sprite(tex_shader, tex_obj);

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

    ImGui::Begin("Game Info", NULL, ImGuiWindowFlags_NoFocusOnAppearing);
    ImGui::Text("player x comp %f y comp %f", x, y);
    ImGui::Text("player pos %f %f", player.transform.position.x, player.transform.position.y);
    ImGui::Text("player vel x: %f y: %f", player.velocity.x, player.velocity.y);
    ImGui::Text("player angle %f", player.transform.angle);
    ImGui::End();

    // ImGui demo window
    // ImGui::ShowDemoWindow(&show_imgui_demo_window);

    if (false)
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