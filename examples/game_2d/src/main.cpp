// -- Resources --
// https://github.com/Turtwiggy/Dwarf-and-Blade/blob/master/src/sprite_renderer.cpp
// https://github.com/Turtwiggy/Dwarf-and-Blade/tree/master/src

// c++ lib headers
#include <iostream>
#include <memory>
#include <vector>

// other library headers
#include <box2d/box2d.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/compatibility.hpp>
#include <glm/gtx/vector_angle.hpp>

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

struct GameObject2D
{
  std::string name = "";
  glm::vec2 pos = { 0.0f, 0.0f }; // in pixels, centered
  float angle_radians = 0.0f;
  glm::vec2 size = { 100.0f, 100.0f };
  glm::vec4 colour = { 1.0f, 1.0f, 1.0f, 1.0f };
  glm::vec2 velocity = { 0.0f, 0.0f };

  int tex_slot = 0;
};

// simple aabb collision
bool
check_collides(GameObject2D& one, GameObject2D& two)
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

float screen_width = 1366.0f;
float screen_height = 768.0f;

// https://colorhunt.co/palette/273312
const glm::vec4 PALETTE_COLOUR_1_0 = glm::vec4(255.0f / 255.0f, 201.0f / 255.0f, 150.0f / 255.0f, 1.0f); // yellowish
const glm::vec4 PALETTE_COLOUR_2_0 = glm::vec4(255.0f / 255.0f, 132.0f / 255.0f, 116.0f / 255.0f, 1.0f); // orange
const glm::vec4 PALETTE_COLOUR_3_0 = glm::vec4(159.0f / 255.0f, 95.0f / 255.0f, 128.0f / 255.0f, 1.0f);  // brown-red
const glm::vec4 PALETTE_COLOUR_4_0 = glm::vec4(88.0f / 255.0f, 61.0f / 255.0f, 114.0f / 255.0f, 1.0f);   // purple

const glm::vec4 PALETTE_COLOUR_1_1 = glm::vec4(57.0f / 255.0f, 62.0f / 255.0f, 70.0f / 255.0f, 1.0f);    // black
const glm::vec4 PALETTE_COLOUR_2_1 = glm::vec4(0.0f / 255.0f, 173.0f / 255.0f, 181.0f / 255.0f, 1.0f);   // blue
const glm::vec4 PALETTE_COLOUR_3_1 = glm::vec4(170.0f / 255.0f, 216.0f / 255.0f, 211.0f / 255.0f, 1.0f); // lightblue
const glm::vec4 PALETTE_COLOUR_4_1 = glm::vec4(238.0f / 255.0f, 238.0f / 255.0f, 238.0f / 255.0f, 1.0f); // grey

glm::vec4 chosen_colour_0 = PALETTE_COLOUR_1_1;
glm::vec4 chosen_colour_1 = PALETTE_COLOUR_2_1;
glm::vec4 chosen_colour_2 = PALETTE_COLOUR_3_1;
glm::vec4 chosen_colour_3 = PALETTE_COLOUR_4_1;

const int tex_unit_kenny_nl = 0;

int
main()
{
  std::cout << "booting up..." << std::endl;
  const auto app_start = std::chrono::high_resolution_clock::now();

  Application app("2D Game", static_cast<int>(screen_width), static_cast<int>(screen_height));
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
  RenderCommand::set_clear_colour(chosen_colour_0);
  RenderCommand::set_viewport(0, 0, static_cast<uint32_t>(screen_width), static_cast<uint32_t>(screen_height));
  RenderCommand::set_depth_testing(false); // disable depth testing for 2d

  Shader sprite_shader = Shader("2d_texture.vert", "2d_spritesheet.frag");
  sprite_shader.bind();
  sprite_shader.set_mat4("projection", projection);
  sprite_shader.set_int("tex", tex_unit_kenny_nl);

  Shader tex_shader = Shader("2d_texture.vert", "2d_texture.frag");
  tex_shader.bind();
  tex_shader.set_mat4("projection", projection);
  tex_shader.set_int("tex", tex_unit_kenny_nl);

  Shader colour_shader = Shader("2d_basic.vert", "2d_colour.frag");
  colour_shader.bind();
  colour_shader.set_vec4("colour", chosen_colour_1);

  sprite::spritemap spritemap;
  auto& sprites = spritemap.get_locations();

  //
  // Game
  //

  GameState state = GameState::GAME_ACTIVE;
  std::vector<GameObject2D> objects;
  float spawn_every = 1.0f;
  float spawn_every_cooldown = 0.0f;

  GameObject2D player;
  player.angle_radians = 0.0;
  player.colour = chosen_colour_1;
  player.size = { 1.0f * 768.0f / 48.0f, 1.0f * 362.0f / 22.0f };
  player.pos = { screen_width / 2.0f, screen_height / 2.0f };
  player.tex_slot = tex_unit_kenny_nl;
  player.velocity = { 5.0f, 5.0f };

  GameObject2D tex_obj;
  tex_obj.angle_radians = 0.0f;
  tex_obj.colour = { 1.0f, 1.0f, 1.0f, 1.0f };
  tex_obj.size = { 768.0f, 352.0f };
  tex_obj.pos = { 0.0f, 20.0f };
  tex_obj.tex_slot = tex_unit_kenny_nl;
  tex_obj.velocity = { 0.0f, 0.0f };

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
  GameObject2D floaty_object;
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
      colour_shader.bind();
      colour_shader.set_mat4("projection", projection);
    }

    //
    // Shader hot reloading
    //
    // if (app.get_input().get_key_held(SDL_SCANCODE_R)) {
    //   reload_shader_program(&sprite_shader.ID, "2d_texture.vert", "2d_spritesheet.frag");
    //   sprite_shader.bind();
    //   sprite_shader.set_mat4("projection", projection);
    //   sprite_shader.set_int("tex", tex_unit_kenny_nl);
    //   sprite_shader.set_int("desired_x", 1);
    //   sprite_shader.set_int("desired_x", 0);
    // }

    //
    // Game: Pause
    //
    if (app.get_input().get_key_down(SDL_SCANCODE_P)) {
      state = state == GameState::GAME_PAUSED ? GameState::GAME_ACTIVE : GameState::GAME_PAUSED;
    }
    //
    // Editor: add object
    //
    if (app.get_input().get_mouse_lmb_down()) {
      glm::ivec2 mouse_pos = app.get_input().get_mouse_pos();
      printf("(game) left mouse clicked %i %i \n", mouse_pos.x, mouse_pos.y);
      glm::vec2 world_pos = glm::vec2(mouse_pos) + camera.pos;
      GameObject2D obj;
      obj.pos = glm::vec2(world_pos.x, world_pos.y);
      objects.push_back(obj);
    }

    profiler.end(Profiler::Stage::SdlInput);
    profiler.begin(Profiler::Stage::GameTick);

    if (state == GameState::GAME_ACTIVE) {

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

      glm::vec2 player_world_space_pos = player.pos - camera.pos;
      mouse_angle_around_player = atan2(app.get_input().get_mouse_pos().y - player_world_space_pos.y,
                                        app.get_input().get_mouse_pos().x - player_world_space_pos.x);
      mouse_angle_around_player += PI / 2.0f;

      bool movement_wasd = true;
      if (movement_wasd) {
        player.velocity = { 1.0f, 1.0f };

        if (app.get_input().get_key_held(SDL_SCANCODE_A)) {
          player.velocity.x = -1.0f;
        } else if (app.get_input().get_key_held(SDL_SCANCODE_D)) {
          player.velocity.x = 1.0f;
        } else {
          player.velocity.x = 0.0f;
        }

        if (app.get_input().get_key_held(SDL_SCANCODE_W)) {
          player.velocity.y = -1.0f;
        } else if (app.get_input().get_key_held(SDL_SCANCODE_S)) {
          player.velocity.y = 1.0f;
        } else {
          player.velocity.y = 0.0f;
        }

        // look in mouse direction
        player.angle_radians = mouse_angle_around_player;
        // look in velocity direction
        // if (glm::length2(player.velocity) > 0) {
        //   glm::vec2 up_axis = glm::vec2(0.0, -1.0);
        //   float unsigned_angle = glm::angle(up_axis, player.velocity);
        //   float sign = (up_axis.x * player.velocity.y - up_axis.y * player.velocity.x) >= 0.0f ? 1.0f : -1.0f;
        //   float signed_angle = unsigned_angle * sign;
        //   player.angle_radians = signed_angle;
        // }
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

        GameObject2D obj;
        obj.name = "BULLET";
        obj.pos = player.pos;
        obj.angle_radians = player.angle_radians;
        obj.velocity = player.velocity;
        objects.push_back(obj);
      }

      // Do Collisions
      std::vector<GameObject2D>::iterator it = objects.begin();
      while (it != objects.end()) {

        GameObject2D& obj = *it;
        bool is_bullet = obj.name == std::string("BULLET"); // TODO: think about layer collisions
        if (!is_bullet && check_collides(player, obj)) {
          printf("Ahh! you are colliding with something");
          it = objects.erase(it);
        } else {
          ++it;
        }
      }

      //
      // Update everything's position
      //

      // Ability: Boost
      float player_speed = 80.0f;
      if (app.get_input().get_key_held(SDL_SCANCODE_LSHIFT))
        player_speed *= 2.0f;

      // player pos
      player.pos += player.velocity * player_speed * delta_time_s;

      // spawned objects pos
      float obj_speed = 50.0f;
      for (int i = 0; i < objects.size(); i++) {
        GameObject2D& obj = objects[i];
        obj.pos += obj.velocity * obj_speed * delta_time_s;
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
    RenderCommand::set_clear_colour(chosen_colour_0);
    RenderCommand::clear();

    tex_shader.bind();
    sprite_renderer::draw_sprite(camera,
                                 glm::ivec2(screen_width, screen_height),
                                 tex_shader,
                                 tex_obj.pos,
                                 tex_obj.size,
                                 tex_obj.angle_radians,
                                 tex_obj.colour);

    glm::ivec2 obj = spritemap.get_sprite_offset(sprite::type::TREE_1);
    sprite_shader.bind();
    sprite_shader.set_int("desired_x", obj.x);
    sprite_shader.set_int("desired_y", obj.y);

    for (auto& object : objects) {
      glm::vec2 size = glm::vec2(50.0f, 50.0f);
      glm::vec3 colour = chosen_colour_2;

      float x = glm::sin(object.angle_radians) * object.velocity.x;
      float y = -glm::cos(object.angle_radians) * object.velocity.y;
      object.pos.x += (x)*delta_time_s;
      object.pos.y += (y)*delta_time_s;

      sprite_renderer::draw_sprite(
        camera, glm::ivec2(screen_width, screen_height), sprite_shader, object.pos, size, object.angle_radians, colour);
    }
    for (auto& pos : points) {
      glm::vec2 size = glm::vec2(50.0f, 50.0f);
      glm::vec3 colour = chosen_colour_2;
      sprite_renderer::draw_sprite(
        camera, glm::vec2(screen_width, screen_height), sprite_shader, pos, size, 0.0f, colour);
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

    // draw lines
    // colour_shader.bind();
    // colour_shader.set_vec4("colour", chosen_colour_3);
    // glBindVertexArray(line_vao);
    // glDrawArrays(GL_LINES, 0, 4);
    // glBindVertexArray(0);

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
    ImGui::Text("player vel x: %f y: %f", player.velocity.x, player.velocity.y);
    ImGui::Text("player angle %f", player.angle_radians);
    ImGui::Text("camera pos %f %f", camera.pos.x, camera.pos.y);
    ImGui::Text("mouse pos %f %f", app.get_input().get_mouse_pos().x, app.get_input().get_mouse_pos().y);
    ImGui::Text("mouse angle around player %f", mouse_angle_around_player);
    ImGui::Text("Spawned objects: %i", objects.size());
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