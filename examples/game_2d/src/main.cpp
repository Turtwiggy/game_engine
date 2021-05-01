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
#include <imgui.h>

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
#include "console.hpp"
#include "sprite_renderer.hpp"
#include "spritemap.hpp"
#include "util.hpp"
using namespace game2d;

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

bool
collides(CollisionLayer& y_l1, CollisionLayer& x_l2)
{
  int c1 = static_cast<int>(y_l1); // c1 is 0, 1, or 2
  int c2 = static_cast<int>(x_l2); // c2 is 0, 1, or 2

  int x_max = 4;
  int val = c1 * x_max + c2;
  // e.g. default, player. y=0, x=1.

  static const std::vector<bool> collision_matrix = {
    true, // default_default_0_0
    true, // default_player_0_1
    true, // default_bullet_0_2
    true, // default_detroyable

    true, // player_default_1_0
    true, // player_player_1_1
    true, // player_bullet_1_2
    true, // player_destroyable_1_2

    true, // bullet_default_2_0
    true, // bullet_player_2_1
    true, // bullet_bullet_2_2
    true, // bullet_destroyable_2_2

    true, // destroyable_default
    true, // destroyable_player
    true, // destroyable_bullet
    true, // destroyable_destroyable
  };

  return collision_matrix[val];
}

enum class GameState
{
  GAME_ACTIVE,
  GAME_MENU,
  GAME_PAUSED
};

// https://colorhunt.co/palette/273312
const glm::vec4 PALETTE_COLOUR_1_0 = glm::vec4(255.0f / 255.0f, 201.0f / 255.0f, 150.0f / 255.0f, 1.0f); // yellowish
const glm::vec4 PALETTE_COLOUR_2_0 = glm::vec4(255.0f / 255.0f, 132.0f / 255.0f, 116.0f / 255.0f, 1.0f); // orange
const glm::vec4 PALETTE_COLOUR_3_0 = glm::vec4(159.0f / 255.0f, 95.0f / 255.0f, 128.0f / 255.0f, 1.0f);  // brown-red
const glm::vec4 PALETTE_COLOUR_4_0 = glm::vec4(88.0f / 255.0f, 61.0f / 255.0f, 114.0f / 255.0f, 1.0f);   // purple

const glm::vec4 PALETTE_COLOUR_1_1 = glm::vec4(57.0f / 255.0f, 62.0f / 255.0f, 70.0f / 255.0f, 1.0f);    // black
const glm::vec4 PALETTE_COLOUR_2_1 = glm::vec4(0.0f / 255.0f, 173.0f / 255.0f, 181.0f / 255.0f, 1.0f);   // blue
const glm::vec4 PALETTE_COLOUR_3_1 = glm::vec4(170.0f / 255.0f, 216.0f / 255.0f, 211.0f / 255.0f, 1.0f); // lightblue
const glm::vec4 PALETTE_COLOUR_4_1 = glm::vec4(238.0f / 255.0f, 238.0f / 255.0f, 238.0f / 255.0f, 1.0f); // grey

float screen_width = 1366.0f;
float screen_height = 768.0f;
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
  Console console;
  bool show_profiler = false;
  bool show_console = false;
  bool show_demo_window = false;

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

  // colours
  glm::vec4 chosen_colour_0 = PALETTE_COLOUR_1_1;
  glm::vec4 chosen_colour_1 = PALETTE_COLOUR_2_1;
  glm::vec4 chosen_colour_2 = PALETTE_COLOUR_3_1;
  glm::vec4 chosen_colour_3 = PALETTE_COLOUR_4_1;
  glm::vec4 background_colour = chosen_colour_0;
  glm::vec4 player_colour = chosen_colour_1;
  glm::vec4 bullet_colour = chosen_colour_2;
  glm::vec4 wall_colour = chosen_colour_3;
  // sprites
  sprite::type player_sprite = sprite::type::TREE_1;
  sprite::type bullet_sprite = sprite::type::TREE_1;
  sprite::type wall_sprite = sprite::type::WALL_BIG;

  // Rendering
  RenderCommand::init();
  RenderCommand::set_clear_colour(background_colour);
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
  std::vector<int> objects_that_collided;
  int objects_collected = 0;

  GameObject2D player;
  player.name = "player";
  player.pos = { screen_width / 2.0f, screen_height / 2.0f };
  player.angle_radians = 0.0;
  player.size = { 1.0f * 768.0f / 48.0f, 1.0f * 362.0f / 22.0f };
  player.colour = player_colour;
  player.velocity = { 0.0f, 0.0f };
  player.sprite = player_sprite;
  player.collision_layer = CollisionLayer::Player;
  player.tex_slot = tex_unit_kenny_nl;

  GameObject2D bullet;
  bullet.name = "bullet";
  bullet.pos = { 0.0f, 0.0f };
  bullet.angle_radians = 0.0;
  bullet.size = { 25.0f, 25.0f };
  bullet.colour = bullet_colour;
  bullet.velocity = { 0.0f, 0.0f };
  bullet.sprite = bullet_sprite;
  bullet.collision_layer = CollisionLayer::Bullet;
  bullet.tex_slot = tex_unit_kenny_nl;

  GameObject2D wall;
  wall.name = "wall";
  wall.pos = { 0.0f, 0.0f };
  wall.angle_radians = 0.0;
  wall.size = { 20.0f, 20.0f };
  wall.colour = wall_colour;
  wall.velocity = { 0.0f, 0.0f };
  wall.sprite = wall_sprite;
  wall.collision_layer = CollisionLayer::Destroyable;
  wall.tex_slot = tex_unit_kenny_nl;

  GameObject2D tex_obj;
  tex_obj.name = "texture_sheet";
  tex_obj.pos = { 0.0f, 20.0f };
  tex_obj.angle_radians = 0.0f;
  tex_obj.size = { 768.0f, 352.0f };
  tex_obj.colour = { 1.0f, 1.0f, 1.0f, 1.0f };
  tex_obj.velocity = { 0.0f, 0.0f };
  tex_obj.tex_slot = tex_unit_kenny_nl;

  float mouse_angle_around_player = 0.0f;

  // ---- App ----

  while (app.is_running()) {

    profiler.new_frame();
    profiler.begin(Profiler::Stage::UpdateLoop);
    profiler.begin(Profiler::Stage::SdlInput);

    app.frame_begin(); // input events
    float delta_time_s = app.get_delta_time();

    if (app.get_input().get_key_down(SDL_SCANCODE_ESCAPE))
      app.shutdown();
    if (app.get_input().get_key_down(SDL_SCANCODE_F12))
      show_console = !show_console;

    //
    // Editor: add object
    //
    if (app.get_input().get_mouse_mmb_down()) {
      glm::ivec2 mouse_pos = app.get_input().get_mouse_pos();
      printf("(game) mmb clicked %i %i \n", mouse_pos.x, mouse_pos.y);
      glm::vec2 world_pos = glm::vec2(mouse_pos) + camera.pos;

      GameObject2D wall_copy;
      // defaults
      wall_copy.name = wall.name;
      wall_copy.angle_radians = wall.angle_radians;
      wall_copy.size = wall.size;
      wall_copy.colour = wall_colour;
      wall_copy.velocity = wall.velocity;
      wall_copy.sprite = wall.sprite;
      wall_copy.collision_layer = wall.collision_layer;
      wall_copy.tex_slot = wall.tex_slot;
      // override defaults
      wall_copy.pos = world_pos;

      objects.push_back(wall_copy);
    }
    //
    // Ability: Boost
    //
    float player_speed = 50.0f;
    if (app.get_input().get_key_held(SDL_SCANCODE_LSHIFT))
      player_speed *= 2.0f;

    profiler.end(Profiler::Stage::SdlInput);
    profiler.begin(Profiler::Stage::GameTick);
    {
      if (state == GameState::GAME_ACTIVE) {

        // camera lrud (standard)
        const float camera_speed = 100.0f;
        if (app.get_input().get_key_held(SDL_SCANCODE_LEFT))
          camera.pos.x -= delta_time_s * camera_speed;
        if (app.get_input().get_key_held(SDL_SCANCODE_RIGHT))
          camera.pos.x += delta_time_s * camera_speed;
        if (app.get_input().get_key_held(SDL_SCANCODE_UP))
          camera.pos.y -= delta_time_s * camera_speed;
        if (app.get_input().get_key_held(SDL_SCANCODE_DOWN))
          camera.pos.y += delta_time_s * camera_speed;

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

        //
        // Ability: Shoot
        //
        if (app.get_input().get_mouse_rmb_down()) {
          float bullet_speed = 50.0f;

          GameObject2D bullet_copy;
          // defaults
          bullet_copy.name = bullet.name;
          bullet_copy.size = bullet.size;
          bullet_copy.colour = bullet.colour;
          bullet_copy.sprite = bullet.sprite;
          bullet_copy.tex_slot = bullet.tex_slot;
          bullet_copy.collision_layer = bullet.collision_layer;
          // override defaults
          bullet_copy.pos = player.pos;
          bullet_copy.angle_radians = mouse_angle_around_player;
          bullet_copy.velocity.x = bullet_speed;
          bullet_copy.velocity.y = bullet_speed;

          objects.push_back(bullet_copy);
        }

        // Do Collisions
        // std::vector<GameObject2D>::iterator it_1 = objects.begin();
        // std::vector<GameObject2D>::iterator it_2 = objects.begin();
        // while (it_1 != objects.end()) {
        //   while (it_2 != objects.end()) {
        //     GameObject2D& obj_1 = *it_1;
        //     GameObject2D& obj_2 = *it_2;
        //     if (obj_1.id == obj_2.id) {
        //       ++it_2;
        //       continue;
        //     }

        //     bool collision_config = collides(obj_1.collision_layer, obj_2.collision_layer);
        //     if (collision_config && check_collides(obj_1, obj_2)) {
        //       std::cout << "collideing objects... " << obj_1.id << "," << obj_2.id << std::endl;
        //       objects_that_collided.push_back(obj_1.id);
        //       objects_that_collided.push_back(obj_2.id);
        //     }
        //     ++it_2;
        //   }
        //   ++it_1;
        // }

        //
        // Update everything's position
        //
        // pos: player
        player.pos += player.velocity * player_speed * delta_time_s;
        // pos: objects
        for (GameObject2D& obj : objects) {
          float x = glm::sin(obj.angle_radians) * obj.velocity.x;
          float y = -glm::cos(obj.angle_radians) * obj.velocity.y;
          obj.pos.x += x * delta_time_s;
          obj.pos.y += y * delta_time_s;
        }
        // pos: camera
        if (app.get_input().get_key_held(SDL_SCANCODE_Q))
          camera.pos = glm::vec2(player.pos.x - screen_width / 2.0f, player.pos.y - screen_height / 2.0f);
      }
    }
    profiler.end(Profiler::Stage::GameTick);
    profiler.begin(Profiler::Stage::Render);
    //
    // rendering
    //
    {
      RenderCommand::set_clear_colour(background_colour);
      RenderCommand::clear();
      glm::ivec2 screen_wh = glm::ivec2(screen_width, screen_height);

      // draw: texture object
      tex_shader.bind();
      sprite_renderer::draw_sprite_debug(camera, screen_wh, tex_shader, tex_obj, colour_shader, chosen_colour_3);

      // draw: game objects
      glm::ivec2 obj;
      for (GameObject2D& object : objects) {
        obj = spritemap.get_sprite_offset(object.sprite);
        sprite_shader.bind();
        sprite_shader.set_int("desired_x", obj.x);
        sprite_shader.set_int("desired_y", obj.y);
        sprite_renderer::draw_sprite_debug(camera, screen_wh, sprite_shader, object, colour_shader, chosen_colour_3);
      }

      // draw: player
      obj = spritemap.get_sprite_offset(player.sprite);
      sprite_shader.bind();
      sprite_shader.set_int("desired_x", obj.x);
      sprite_shader.set_int("desired_y", obj.y);
      sprite_renderer::draw_sprite_debug(camera, screen_wh, sprite_shader, player, colour_shader, chosen_colour_3);
    }
    profiler.end(Profiler::Stage::Render);
    profiler.begin(Profiler::Stage::GuiLoop);
    //
    // GUI
    //
    {
      if (ImGui::BeginMainMenuBar()) {
        if (ImGui::MenuItem("Quit", "Esc"))
          app.shutdown();
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
      ImGui::Text("GameObjects: %i", objects.size());
      ImGui::Text("Collectables: %i", objects_collected);
      ImGui::End();

      ImGui::Begin("Objects", NULL, ImGuiWindowFlags_NoFocusOnAppearing);
      for (GameObject2D& go : objects) {
        ImGui::Text("ID: %i, Name: %s", go.id, go.name.c_str());
      }
      ImGui::End();

      ImGui::Begin("Collided Objects", NULL, ImGuiWindowFlags_NoFocusOnAppearing);
      for (int go : objects_that_collided) {
        ImGui::Text("ID: %i", go);
      }
      ImGui::End();

      if (show_console)
        console.Draw("Console", &show_console);
      if (show_profiler)
        profiler_panel::draw(profiler, delta_time_s);
      if (show_demo_window)
        ImGui::ShowDemoWindow(&show_demo_window);
    }
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
// Settings: Fullscreen
//
// if (app.get_input().get_key_down(SDL_SCANCODE_F)) {
//   app.get_window().toggle_fullscreen(); // SDL2 window toggle
//   glm::ivec2 screen_size = app.get_window().get_size();
//   RenderCommand::set_viewport(0, 0, screen_size.x, screen_size.y);

//   screen_width = static_cast<float>(screen_size.x);
//   screen_height = static_cast<float>(screen_size.y);
//   projection = glm::ortho(0.0f, screen_width, screen_height, 0.0f, -1.0f, 1.0f);
//   sprite_shader.bind();
//   sprite_shader.set_mat4("projection", projection);
//   tex_shader.bind();
//   tex_shader.set_mat4("projection", projection);
//   colour_shader.bind();
//   colour_shader.set_mat4("projection", projection);
// }

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
// if (app.get_input().get_key_down(SDL_SCANCODE_P)) {
//   state = state == GameState::GAME_PAUSED ? GameState::GAME_ACTIVE : GameState::GAME_PAUSED;
// }
// // Game Thing: Reset player pos
// if (app.get_input().get_key_held(SDL_SCANCODE_O)) {
//   player.pos = glm::vec2(0.0f, 0.0f);
// }

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