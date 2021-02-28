
// c++ lib headers
#include <iostream>
#include <memory>
#include <vector>

// other library headers
#include <box2d/box2d.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/compatibility.hpp>

// hack: temp
#include <GL/glew.h>

// fightingengine headers
#include "engine/application.hpp"
#include "engine/maths_core.hpp"
#include "engine/opengl/render_command.hpp"
#include "engine/ui/profiler_panel.hpp"
using namespace fightingengine;

// game headers
#include "camera2d.hpp"
#include "game.hpp"
#include "sprite_renderer.hpp"
#include "spritemap.hpp"
#include "util.hpp"
using namespace game2d;

// other things to consider
// colisions
// particles
// post processing
// rendering text

// -- temp --
// https://github.com/Turtwiggy/Dwarf-and-Blade/blob/master/src/sprite_renderer.cpp
// https://github.com/Turtwiggy/Dwarf-and-Blade/tree/master/src

struct Transform
{
  glm::vec2 position = { 0.0f, 0.0f }; // in pixels, centered
  float angle = 0.0f;                  // in degrees
  glm::vec2 scale = { 100.0f, 100.0f };
  glm::vec4 colour = { 1.0f, 1.0f, 1.0f, 1.0f };
};

struct GameObject
{
  Transform transform;
  glm::vec2 velocity = { 0.0f, 0.0f };
  int tex_slot = 0;

  // gameobject flags...
  // note: avoid adding these, or come up with a better system
  // more flags is a 2^n of configurations of testing to make sure everything
  bool is_solid = false;
  bool destroyed = false;

  GameObject() = default;
};

enum class GameState
{
  GAME_ACTIVE,
  GAME_MENU,
  GAME_PAUSED
};

// -- end temp --

float screen_width = 1366.0f;
float screen_height = 768.0f;

const glm::vec4 dark_blue = glm::vec4(0.0f / 255.0f, 100.0f / 255.0f, 100.0f / 255.0f, 1.0f);
const int tex_unit_kenny_nl = 0;

const float player_speed = 10.0f;
const float camera_speed = 100.0f;

int
main()
{
  std::cout << "booting up..." << std::endl;
  const auto app_start = std::chrono::high_resolution_clock::now();

  Application app("2D Game", screen_width, screen_height);
  // app.set_fps_limit(60.0f);

  RandomState rnd;
  Profiler profiler;

  glm::mat4 projection = glm::ortho(0.0f, screen_width, screen_height, 0.0f, -1.0f, 1.0f);

  Camera2D camera;
  camera.pos = glm::vec2{ 0.0f, 0.0f };

  // textures
  std::vector<std::pair<int, std::string>> textures_to_load;
  textures_to_load.emplace_back(tex_unit_kenny_nl,
                                "assets/textures/kennynl_1bit_pack/Tilesheet/monochrome_transparent_packed.png");
  load_textures_threaded(textures_to_load, app_start);

  //
  // TODO sound
  //

  // Rendering
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
  tex_shader.set_int("tex", tex_unit_kenny_nl);

  sprite::spritemap spritemap;
  auto& sprites = spritemap.get_locations();

  // ---- Game ----

  GameState state = GameState::GAME_ACTIVE;
  std::vector<glm::vec2> objects;

  GameObject player;
  player.transform.angle = 90.0f;
  player.transform.colour = { 0.0f, 1.0f, 0.0f, 0.6f };
  player.transform.scale = { 1.0f * 768.0f / 48.0f, 1.0f * 362.0f / 22.0f };
  player.transform.position = { screen_width / 2.0f, screen_height / 2.0f };
  player.tex_slot = tex_unit_kenny_nl;
  player.velocity = { 5.0f, 5.0f };

  // player things
  float boop_cooldown = 1.0f;
  bool l_boop = false;
  float l_boop_cooldown_left = 1.0f;
  bool r_boop = false;
  float r_boop_cooldown_left = 1.0f;

  GameObject tex_obj;
  tex_obj.transform.angle = 0.0f;
  tex_obj.transform.colour = { 1.0f, 1.0f, 1.0f, 1.0f };
  tex_obj.transform.scale = { 768.0f, 352.0f };
  tex_obj.transform.position = { 0.0f, 20.0f };
  tex_obj.tex_slot = tex_unit_kenny_nl;
  tex_obj.velocity = { 0.0f, 0.0f };

  // four random points on the screen
  float offsets = 200.0f;
  glm::vec2 tl(screen_width / 2.0f - offsets, screen_height / 2.0f - offsets);
  glm::vec2 tr(screen_width / 2.0f + offsets, screen_height / 2.0f - offsets);
  glm::vec2 br(screen_width / 2.0f + offsets, screen_height / 2.0f + offsets);
  glm::vec2 bl(screen_width / 2.0f - offsets, screen_height / 2.0f + offsets);
  int next_point = 0;
  std::vector<glm::vec2> points{ tl, tr, br, bl };

  float percent = 0.0f;
  GameObject floaty_object;
  floaty_object.transform.position = points[0];
  glm::vec2 floaty_object_destination = points[1];
  floaty_object.tex_slot = tex_unit_kenny_nl;

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
    profiler.begin(Profiler::Stage::SdlInput);

    app.frame_begin(); // input events

    float delta_time_s = app.get_delta_time();

    //
    // Settings: Shutdown app
    //
    if (app.get_input().get_key_down(SDL_SCANCODE_ESCAPE))
      app.shutdown();

    //
    // Settings: Fullscreen
    //
    if (app.get_input().get_key_down(SDL_SCANCODE_F)) {
      app.get_window().toggle_fullscreen(); // SDL2 window toggle
      glm::ivec2 screen_size = app.get_window().get_size();
      RenderCommand::set_viewport(0, 0, screen_size.x, screen_size.y);

      screen_width = screen_size.x;
      screen_height = screen_size.y;
      projection = glm::ortho(0.0f, screen_width, screen_height, 0.0f, -1.0f, 1.0f);
      sprite_shader.bind();
      sprite_shader.set_mat4("projection", projection);
      tex_shader.bind();
      tex_shader.set_mat4("projection", projection);
    }

    //
    // Shader hot reloading
    //
    if (app.get_input().get_key_held(SDL_SCANCODE_R)) {
      reload_shader_program(&sprite_shader.ID, "2d_texture.vert", "2d_spritesheet.frag");
      sprite_shader.bind();
      sprite_shader.set_mat4("projection", projection);
      sprite_shader.set_int("tex", tex_unit_kenny_nl);
      sprite_shader.set_int("desired_x", 1);
      sprite_shader.set_int("desired_x", 0);
    }

    // Game Thing: Pause
    if (app.get_input().get_key_down(SDL_SCANCODE_P)) {
      state = GameState::GAME_PAUSED;
    }
    if (app.get_input().get_key_up(SDL_SCANCODE_P)) {
      state = GameState::GAME_ACTIVE;
    }

    // Game Thing: Reset player pos
    if (app.get_input().get_key_held(SDL_SCANCODE_O)) {
      player.transform.position = glm::vec2(0.0f, 0.0f);
    }

    // Reset Camera
    // if (app.get_input().get_key_held(SDL_SCANCODE_E))
    camera.pos =
      glm::vec2(player.transform.position.x - screen_width / 2.0f, player.transform.position.y - screen_height / 2.0f);

    //
    // Editor: Add object
    //
    if (app.get_input().get_mouse_lmb_down()) {
      glm::ivec2 mouse_pos = app.get_input().get_mouse_pos();
      printf("(game) left mouse clicked %i %i \n", mouse_pos.x, mouse_pos.y);

      glm::vec2 world_pos = glm::vec2(mouse_pos) + camera.pos;

      objects.push_back(glm::vec2(world_pos.x, world_pos.y));
    }

    // if (app.get_input().get_key_down(SDL_KeyCode::SDLK_t))
    //   dynamic_body->SetTransform(b2Vec2(0.0f, 0.0f), dynamic_body->GetAngle());

    // if (app.get_input().get_mouse_lmb_held()) {
    //   glm::ivec2 mouse_pos = app.get_window().get_mouse_position();
    //   dynamic_body->SetTransform(b2Vec2(mouse_pos.x, mouse_pos.y), dynamic_body->GetAngle());
    // }

    profiler.end(Profiler::Stage::SdlInput);
    profiler.begin(Profiler::Stage::GameTick);

    if (state == GameState::GAME_ACTIVE) {

      float camera_velocity_x = delta_time_s * camera_speed;
      float camera_velocity_y = delta_time_s * camera_speed;

      // camera lrud (standard)
      if (app.get_input().get_key_held(SDL_SCANCODE_LEFT))
        camera.pos.x -= camera_velocity_x;
      if (app.get_input().get_key_held(SDL_SCANCODE_RIGHT))
        camera.pos.x += camera_velocity_x;
      if (app.get_input().get_key_held(SDL_SCANCODE_UP))
        camera.pos.y -= camera_velocity_y;
      if (app.get_input().get_key_held(SDL_SCANCODE_DOWN))
        camera.pos.y += camera_velocity_y;

      // spaceship lrud

      float angle_speed = offsets;
      if (app.get_input().get_key_held(SDL_SCANCODE_W))
        player.velocity.y += player_speed;
      if (app.get_input().get_key_held(SDL_SCANCODE_S))
        player.velocity.y -= player_speed;

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

      // Turn Ship
      if (app.get_input().get_key_held(SDL_SCANCODE_D))
        player.transform.angle += delta_time_s * angle_speed;
      if (app.get_input().get_key_held(SDL_SCANCODE_A))
        player.transform.angle -= delta_time_s * angle_speed;

      float extra_x = 0.0f;
      float extra_y = 0.0f;
      float boop_amount = 5000.0f;

      if (app.get_input().get_key_down(SDL_SCANCODE_A) && l_boop) {
        l_boop = false;
        extra_x = glm::sin(glm::radians(player.transform.angle - 90.0f)) * boop_amount;
        extra_y = -glm::cos(glm::radians(player.transform.angle - 90.0f)) * boop_amount;
      } else if (app.get_input().get_key_down(SDL_SCANCODE_A)) {
        l_boop = true;
        l_boop_cooldown_left = boop_cooldown;
      }
      if (l_boop_cooldown_left > 0.0f)
        l_boop_cooldown_left -= delta_time_s;
      else
        l_boop = false;

      if (app.get_input().get_key_down(SDL_SCANCODE_D) && r_boop) {
        r_boop = false;
        extra_x = glm::sin(glm::radians(player.transform.angle + 90.0f)) * boop_amount;
        extra_y = -glm::cos(glm::radians(player.transform.angle + 90.0f)) * boop_amount;
      } else if (app.get_input().get_key_down(SDL_SCANCODE_D)) {
        r_boop = true;
        r_boop_cooldown_left = boop_cooldown;
      }
      if (r_boop_cooldown_left > 0.0f)
        r_boop_cooldown_left -= delta_time_s;
      else
        r_boop = false;

      // update get vector based on angle
      float x = glm::sin(glm::radians(player.transform.angle)) * turn_velocity_x;
      float y = -glm::cos(glm::radians(player.transform.angle)) * turn_velocity_y;

      player.transform.position.x += (x + extra_x) * delta_time_s;
      player.transform.position.y += (y + extra_y) * delta_time_s;

      // move floaty object to point
      floaty_object.transform.position = glm::lerp(points[next_point], points[(next_point + 1) % 4], percent);
      float time_to_take_seconds = 4.0f;
      percent += delta_time_s / time_to_take_seconds;
      if (percent >= 1.0f) {
        percent = 0.0f;
        next_point += 1;
        next_point %= 4;
      }
    }

    profiler.end(Profiler::Stage::GameTick);
    profiler.begin(Profiler::Stage::Render);
    //
    // rendering
    //
    RenderCommand::set_clear_colour(dark_blue);
    RenderCommand::clear();

    tex_shader.bind();
    sprite_renderer::draw_sprite(camera,
                                 glm::ivec2(screen_width, screen_height),
                                 tex_shader,
                                 tex_obj.transform.position,
                                 tex_obj.transform.scale,
                                 tex_obj.transform.angle,
                                 tex_obj.transform.colour);

    glm::ivec2 obj = spritemap.get_sprite_offset(sprite::type::TREE_1);
    sprite_shader.bind();
    sprite_shader.set_int("desired_x", obj.x);
    sprite_shader.set_int("desired_y", obj.y);

    for (auto& object : objects) {
      glm::vec2 size = glm::vec2(50.0f, 50.0f);
      glm::vec3 colour = glm::vec3(1.0f, 0.0f, 0.0f);
      sprite_renderer::draw_sprite(
        camera, glm::ivec2(screen_width, screen_height), sprite_shader, object, size, 0.0f, colour);
    }
    for (auto& object : points) {
      glm::vec2 size = glm::vec2(50.0f, 50.0f);
      glm::vec3 colour = glm::vec3(1.0f, 0.0f, 0.0f);
      sprite_renderer::draw_sprite(
        camera, glm::vec2(screen_width, screen_height), sprite_shader, object, size, 0.0f, colour);
    }

    sprite_renderer::draw_sprite(camera,
                                 glm::ivec2(screen_width, screen_height),
                                 sprite_shader,
                                 floaty_object.transform.position,
                                 floaty_object.transform.scale,
                                 floaty_object.transform.angle,
                                 floaty_object.transform.colour);

    obj = spritemap.get_sprite_offset(sprite::type::TREE_1);
    sprite_shader.set_int("desired_x", obj.x);
    sprite_shader.set_int("desired_y", obj.y);
    sprite_renderer::draw_sprite(camera,
                                 glm::ivec2(screen_width, screen_height),
                                 sprite_shader,
                                 player.transform.position,
                                 player.transform.scale,
                                 player.transform.angle,
                                 player.transform.colour);

    profiler.end(Profiler::Stage::Render);
    profiler.begin(Profiler::Stage::GuiLoop);
    //
    // GUI
    //
    if (ImGui::BeginMainMenuBar()) {

      if (ImGui::MenuItem("Quit", "Esc")) {
        app.shutdown();
      }

      ImGui::SameLine(ImGui::GetWindowWidth() - 154.0f);
      ImGui::Text("%.2f FPS (%.2f ms)", ImGui::GetIO().Framerate, 1000.0f / ImGui::GetIO().Framerate);

      ImGui::EndMainMenuBar();
    }

    ImGui::Begin("Game Info", NULL, ImGuiWindowFlags_NoFocusOnAppearing);
    ImGui::Text("player pos %f %f", player.transform.position.x, player.transform.position.y);
    ImGui::Text("player vel x: %f y: %f", player.velocity.x, player.velocity.y);
    ImGui::Text("player angle %f", player.transform.angle);
    ImGui::Text("camera pos %f %f", camera.pos.x, camera.pos.y);
    ImGui::Text("mouse pos %f %f", app.get_input().get_mouse_pos().x, app.get_input().get_mouse_pos().y);
    ImGui::Text("lboop %d rboop %d", l_boop, r_boop);
    ImGui::End();

    // ImGui demo window
    // ImGui::ShowDemoWindow(&show_imgui_demo_window);

    if (false)
      profiler_panel::draw(profiler, delta_time_s);

    profiler.end(Profiler::Stage::GuiLoop);
    profiler.begin(Profiler::Stage::FrameEnd);

    // end frame
    app.frame_end(delta_time_s);

    profiler.end(Profiler::Stage::FrameEnd);
    profiler.end(Profiler::Stage::UpdateLoop);
  }
}