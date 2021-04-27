
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

struct GameObject
{
  glm::vec2 pos = { 0.0f, 0.0f }; // in pixels, centered
  float angle_radians = 0.0f;
  glm::vec2 size = { 100.0f, 100.0f };
  glm::vec4 colour = { 1.0f, 1.0f, 1.0f, 1.0f };
  glm::vec2 velocity = { 0.0f, 0.0f };

  int tex_slot = 0;
};

// simple aabb collision
bool
check_collides(GameObject& one, GameObject& two)
{
  // collision x-axis?
  bool collisionX = one.pos.x + one.size.x >= two.pos.x && two.pos.x + two.size.x >= one.pos.x;
  // collision y-axis?
  bool collisionY = one.pos.y + one.size.y >= two.pos.y && two.pos.y + two.size.y >= one.pos.y;
  // collision only if on both axes
  return collisionX && collisionY;
}

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

  Shader sprite_shader = Shader("2d.vert", "2d_spritesheet.frag");
  sprite_shader.bind();
  sprite_shader.set_mat4("projection", projection);
  sprite_shader.set_int("tex", tex_unit_kenny_nl);

  Shader tex_shader = Shader("2d.vert", "2d_texture.frag");
  tex_shader.bind();
  tex_shader.set_mat4("projection", projection);
  tex_shader.set_int("tex", tex_unit_kenny_nl);

  Shader red_shader = Shader("2d_red.vert", "2d_red.frag");
  red_shader.bind();

  sprite::spritemap spritemap;
  auto& sprites = spritemap.get_locations();

  // ---- Game ----

  GameState state = GameState::GAME_ACTIVE;
  std::vector<GameObject> objects;
  float spawn_every = 1.0f;
  float spawn_every_cooldown = 0.0f;

  GameObject player;
  player.angle_radians = 0.0;
  player.colour = { 0.0f, 1.0f, 0.0f, 0.6f };
  player.size = { 1.0f * 768.0f / 48.0f, 1.0f * 362.0f / 22.0f };
  player.pos = { screen_width / 2.0f, screen_height / 2.0f };
  player.tex_slot = tex_unit_kenny_nl;
  player.velocity = { 5.0f, 5.0f };

  // player things

  GameObject tex_obj;
  tex_obj.angle_radians = 0.0f;
  tex_obj.colour = { 1.0f, 1.0f, 1.0f, 1.0f };
  tex_obj.size = { 768.0f, 352.0f };
  tex_obj.pos = { 0.0f, 20.0f };
  tex_obj.tex_slot = tex_unit_kenny_nl;
  tex_obj.velocity = { 0.0f, 0.0f };

  bool move_player = false;
  float mouse_angle_around_player = 0.0f;

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
  floaty_object.pos = points[0];
  glm::vec2 floaty_object_destination = points[1];
  floaty_object.tex_slot = tex_unit_kenny_nl;

  // https://solarianprogrammer.com/2013/05/13/opengl-101-drawing-primitives/
  // https://www.youtube.com/watch?v=D2a5fHX-Qrs&t=2s circle vs rectangle
  // aabb https://learnopengl.com/In-Practice/2D-Game/Collisions/Collision-detection
  // https://www.youtube.com/watch?v=NbSee-XM7WA ray casting using dda

  // clang-format off
  GLfloat line_verts[] = 
  { 
    -screen_width, -screen_height,
    screen_width, screen_height,
    -screen_width, screen_height,
    screen_width, -screen_height,
  };
  // clang-format on
  unsigned int line_vao = 0;
  unsigned int line_vbo;
  // bind
  glGenVertexArrays(1, &line_vao);
  glGenBuffers(1, &line_vbo);
  // load data vbo
  glBindBuffer(GL_ARRAY_BUFFER, line_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(line_verts), line_verts, GL_STATIC_DRAW);
  // load data vao
  glBindVertexArray(line_vao);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
  // unbind
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

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
      red_shader.bind();
      red_shader.set_mat4("projection", projection);
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

    //
    // Game: Pause
    //
    if (app.get_input().get_key_down(SDL_SCANCODE_P)) {
      state = state == GameState::GAME_PAUSED ? GameState::GAME_ACTIVE : GameState::GAME_PAUSED;
    }

    //
    // Add object
    //
    if (app.get_input().get_mouse_lmb_down()) {
      glm::ivec2 mouse_pos = app.get_input().get_mouse_pos();
      printf("(game) left mouse clicked %i %i \n", mouse_pos.x, mouse_pos.y);
      glm::vec2 world_pos = glm::vec2(mouse_pos) + camera.pos;
      GameObject obj;
      obj.pos = glm::vec2(world_pos.x, world_pos.y);
      objects.push_back(obj);
    }

    profiler.end(Profiler::Stage::SdlInput);
    profiler.begin(Profiler::Stage::GameTick);

    if (state == GameState::GAME_ACTIVE) {

      // Game Thing: Randomly spawn things
      spawn_every_cooldown += delta_time_s;
      if (spawn_every_cooldown > spawn_every) {

        float x = rand_det_s(rnd.rng, 0.0f, 1.0f);
        float y = rand_det_s(rnd.rng, 0.0f, 1.0f);
        glm::vec2 rand_pos(x * screen_width, y * screen_height);
        printf("random pos: %f %f", rand_pos.x, rand_pos.y);

        GameObject obj;
        obj.pos = rand_pos;
        objects.push_back(obj);

        spawn_every_cooldown = 0.0f;
      }

      // Game Thing: Reset player pos
      if (app.get_input().get_key_held(SDL_SCANCODE_O)) {
        player.pos = glm::vec2(0.0f, 0.0f);
      }

      const float camera_speed = 100.0f;
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
      const float player_speed = 10.0f;

      if (app.get_input().get_key_held(SDL_SCANCODE_W))
        player.velocity.y += player_speed;
      if (app.get_input().get_key_held(SDL_SCANCODE_S))
        player.velocity.y -= player_speed;

      float turn_velocity_x = player.velocity.y; // same as y so turning doesnt feel weird
      float turn_velocity_y = player.velocity.y;

      float angle_speed = 200.0f;
      if (app.get_input().get_key_held(SDL_SCANCODE_SPACE)) {
        turn_velocity_x = 5.0f;
        turn_velocity_y = 5.0f;

        // increase turn speed
        // angle_speed *= 5.0f;
      }

      mouse_angle_around_player =
        atan2(app.get_input().get_mouse_pos().y - player.pos.y, app.get_input().get_mouse_pos().x - player.pos.x);
      mouse_angle_around_player += PI / 2.0f;

      // Turn Ship
      // if (app.get_input().get_key_held(SDL_SCANCODE_D))
      //   player.angle += delta_time_s * angle_speed;
      // if (app.get_input().get_key_held(SDL_SCANCODE_A))
      //   player.angle -= delta_time_s * angle_speed;

      // Boost
      float extra_speed = 1.0f;
      if (app.get_input().get_key_held(SDL_SCANCODE_LSHIFT))
        extra_speed = 2.0f;

      if (move_player) {
        // update get vector based on angle
        // float x = glm::sin(player.angle_radians) * turn_velocity_x * extra_speed;
        // float y = -glm::cos(player.angle_radians) * turn_velocity_y * extra_speed;
        float x = glm::sin(mouse_angle_around_player) * turn_velocity_x * extra_speed;
        float y = -glm::cos(mouse_angle_around_player) * turn_velocity_y * extra_speed;
        player.pos.x += x * delta_time_s;
        player.pos.y += y * delta_time_s;
      }

      // move floaty object to point
      floaty_object.pos = glm::lerp(points[next_point], points[(next_point + 1) % 4], percent);
      float time_to_take_seconds = 4.0f;
      percent += delta_time_s / time_to_take_seconds;
      if (percent >= 1.0f) {
        percent = 0.0f;
        next_point += 1;
        next_point %= 4;
      }

      // Shoot
      if (app.get_input().get_mouse_rmb_down()) {
        glm::ivec2 mouse_pos = app.get_input().get_mouse_pos();
        printf("(game) right mouse clicked %i %i \n", mouse_pos.x, mouse_pos.y);
        glm::vec2 world_pos = glm::vec2(mouse_pos) + camera.pos;

        GameObject obj;
        obj.pos = player.pos;
        obj.angle_radians = player.angle_radians;
        obj.velocity = player.velocity;
        objects.push_back(obj);
      }

      // Do Collisions
      std::vector<GameObject>::iterator it = objects.begin();
      while (it != objects.end()) {

        if (check_collides(player, *it)) {
          printf("Ahh! you are colliding with something");
          it = objects.erase(it);
        } else {
          ++it;
        }
      }

      // Reset Camera
      if (app.get_input().get_key_held(SDL_SCANCODE_Q))
        camera.pos = glm::vec2(player.pos.x - screen_width / 2.0f, player.pos.y - screen_height / 2.0f);
    }

    profiler.end(Profiler::Stage::GameTick);
    profiler.begin(Profiler::Stage::Render);
    //
    // rendering
    //
    RenderCommand::set_clear_colour(dark_blue);
    RenderCommand::clear();

    // tex_shader.bind();
    // sprite_renderer::draw_sprite(camera,
    //                              glm::ivec2(screen_width, screen_height),
    //                              tex_shader,
    //                              tex_obj.pos,
    //                              tex_obj.size,
    //                              tex_obj.angle,
    //                              tex_obj.colour);

    glm::ivec2 obj = spritemap.get_sprite_offset(sprite::type::TREE_1);
    sprite_shader.bind();
    sprite_shader.set_int("desired_x", obj.x);
    sprite_shader.set_int("desired_y", obj.y);

    float extra_speed = 10.0f;
    for (auto& object : objects) {
      glm::vec2 size = glm::vec2(50.0f, 50.0f);
      glm::vec3 colour = glm::vec3(1.0f, 0.0f, 0.0f);

      float x = glm::sin(object.angle_radians) * object.velocity.x;
      float y = -glm::cos(object.angle_radians) * object.velocity.y;
      object.pos.x += (x)*delta_time_s;
      object.pos.y += (y)*delta_time_s;

      sprite_renderer::draw_sprite(
        camera, glm::ivec2(screen_width, screen_height), sprite_shader, object.pos, size, object.angle_radians, colour);
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
                                 floaty_object.pos,
                                 floaty_object.size,
                                 floaty_object.angle_radians,
                                 floaty_object.colour);

    obj = spritemap.get_sprite_offset(sprite::type::TREE_1);
    sprite_shader.set_int("desired_x", obj.x);
    sprite_shader.set_int("desired_y", obj.y);
    sprite_renderer::draw_sprite(camera,
                                 glm::ivec2(screen_width, screen_height),
                                 sprite_shader,
                                 player.pos,
                                 player.size,
                                 player.angle_radians,
                                 player.colour);

    // draw a red line
    red_shader.bind();
    glBindVertexArray(line_vao);
    glDrawArrays(GL_LINES, 0, 4);
    glBindVertexArray(0);

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
    ImGui::Text("player pos %f %f", player.pos.x, player.pos.y);
    ImGui::Text("player accel x: %f y: %f", player.velocity.x, player.velocity.y);
    ImGui::Text("player vel (BROKEN) x: %f y: %f", 0.0f, 0.0f);
    ImGui::Text("player angle %f", player.angle_radians);
    ImGui::Text("camera pos %f %f", camera.pos.x, camera.pos.y);
    ImGui::Text("mouse pos %f %f", app.get_input().get_mouse_pos().x, app.get_input().get_mouse_pos().y);
    ImGui::Text("Spawned objects: %i", objects.size());
    ImGui::Text("Mouse angle around player %f", mouse_angle_around_player);
    ImGui::Checkbox("Move player? ", &move_player);
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

// CODE GRAVEYARD

//
// Boop left or right feature
//

// float boop_cooldown = 1.0f;
// bool l_boop = false;
// float l_boop_cooldown_left = 1.0f;
// bool r_boop = false;
// float r_boop_cooldown_left = 1.0f;

// float extra_x = 0.0f;
// float extra_y = 0.0f;
// // float boop_amount = 5000.0f;
// // if (app.get_input().get_key_down(SDL_SCANCODE_A) && l_boop) {
// //   l_boop = false;
// //   extra_x = glm::sin(glm::radians(player.angle - 90.0f)) * boop_amount;
// //   extra_y = -glm::cos(glm::radians(player.angle - 90.0f)) * boop_amount;
// // } else if (app.get_input().get_key_down(SDL_SCANCODE_A)) {
// //   l_boop = true;
// //   l_boop_cooldown_left = boop_cooldown;
// // }
// // if (l_boop_cooldown_left > 0.0f)
// //   l_boop_cooldown_left -= delta_time_s;
// // else
// //   l_boop = false;

// // if (app.get_input().get_key_down(SDL_SCANCODE_D) && r_boop) {
// //   r_boop = false;
// //   extra_x = glm::sin(glm::radians(player.angle + 90.0f)) * boop_amount;
// //   extra_y = -glm::cos(glm::radians(player.angle + 90.0f)) * boop_amount;
// // } else if (app.get_input().get_key_down(SDL_SCANCODE_D)) {
// //   r_boop = true;
// //   r_boop_cooldown_left = boop_cooldown;
// // }
// // if (r_boop_cooldown_left > 0.0f)
// //   r_boop_cooldown_left -= delta_time_s;
// // else
// //   r_boop = false;