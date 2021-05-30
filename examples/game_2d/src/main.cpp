// -- Resources --
// https://github.com/Turtwiggy/Dwarf-and-Blade/blob/master/src/sprite_renderer.cpp
// https://github.com/Turtwiggy/Dwarf-and-Blade/tree/master/src

// c++ lib headers
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

// other library headers
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/compatibility.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <imgui.h>

// fightingengine headers
#include "engine/application.hpp"
#include "engine/maths_core.hpp"
#include "engine/opengl/render_command.hpp"
#include "engine/ui/profiler_panel.hpp"
#include "engine/util.hpp"
using namespace fightingengine;

// game headers
#include "game/2d_game_object.hpp"
#include "opengl/sprite_renderer.hpp"
#include "systems/2d_physics.hpp"
#include "systems/console.hpp"
#include "systems/spritemap.hpp"
using namespace game2d;

float screen_width = 720.0f;
float screen_height = 480.0f;
bool show_game_info = true;
bool show_profiler = true;
bool show_console = false;
bool show_demo_window = false;
bool advance_one_frame = false;
// key bindings: application
SDL_Scancode key_quit = SDL_SCANCODE_ESCAPE;
SDL_Scancode key_console = SDL_SCANCODE_F12;
SDL_Scancode key_fullscreen = SDL_SCANCODE_F;

enum class GameState
{
  GAME_SPLASH_SCREEN,
  GAME_OVER_SCREEN,
  GAME_ACTIVE,
  GAME_PAUSED
};
GameState state = GameState::GAME_ACTIVE;
const float time_on_game_over_screen = 3.0f;
float time_on_game_over_screen_left = time_on_game_over_screen;
bool first_time_game_over_screen = true;

// textures
const int tex_unit_kenny_nl = 0;
// default colour palette; https://colorhunt.co/palette/273312
const glm::vec4 PALETTE_COLOUR_1_1 = glm::vec4(57.0f / 255.0f, 62.0f / 255.0f, 70.0f / 255.0f, 1.0f);    // black
const glm::vec4 PALETTE_COLOUR_2_1 = glm::vec4(0.0f / 255.0f, 173.0f / 255.0f, 181.0f / 255.0f, 1.0f);   // blue
const glm::vec4 PALETTE_COLOUR_3_1 = glm::vec4(170.0f / 255.0f, 216.0f / 255.0f, 211.0f / 255.0f, 1.0f); // lightblue
const glm::vec4 PALETTE_COLOUR_4_1 = glm::vec4(238.0f / 255.0f, 238.0f / 255.0f, 238.0f / 255.0f, 1.0f); // grey
// chosen colours
glm::vec4 chosen_colour_0 = PALETTE_COLOUR_1_1;
glm::vec4 chosen_colour_1 = PALETTE_COLOUR_2_1;
glm::vec4 chosen_colour_2 = PALETTE_COLOUR_3_1;
glm::vec4 chosen_colour_3 = PALETTE_COLOUR_4_1;
// entity colours
glm::vec4 background_colour = chosen_colour_0;  // black
glm::vec4 debug_line_colour = chosen_colour_1;  // blue
glm::vec4 player_colour = chosen_colour_1;      // blue
glm::vec4 bullet_colour = chosen_colour_2;      // lightblue
glm::vec4 wall_colour = chosen_colour_3;        // grey
glm::vec4 logo_entity_colour = chosen_colour_3; // grey
// entity sprite defaults
sprite::type logo_sprite = sprite::type::WALL_BIG;
sprite::type player_sprite = sprite::type::TREE_1;
sprite::type bullet_sprite = sprite::type::TREE_1;
sprite::type wall_sprite = sprite::type::WALL_BIG;

// entity: bullet
int bullets_to_fire_after_releasing_mouse = 1;
int bullets_to_fire_after_releasing_mouse_left = 0;
float bullet_seconds_between_spawning = 0.15f;
float bullet_seconds_between_spawning_left = 0.0f;
// entity: enemy
float wall_seconds_between_spawning = 0.5f;
float wall_seconds_between_spawning_left = 0.0f;
const float enemy_default_speed = 50.0f;

namespace gameobject {

static void
update_position(GameObject2D& obj, float delta_time_s)
{
  obj.pos += obj.velocity * delta_time_s;
}

}

namespace camera {

GameObject2D
create_camera()
{
  GameObject2D game_object;
  game_object.pos = glm::vec2{ 0.0f, 0.0f };
  return game_object;
}

static void
update_position(GameObject2D& camera, Application& app, float delta_time_s)
{
  // go.pos = glm::vec2(other.pos.x - screen_width / 2.0f, other.pos.y - screen_height / 2.0f);
  if (app.get_input().get_key_held(camera.keys.key_camera_left))
    camera.pos.x -= delta_time_s * camera.speed_current;
  if (app.get_input().get_key_held(camera.keys.key_camera_right))
    camera.pos.x += delta_time_s * camera.speed_current;
  if (app.get_input().get_key_held(camera.keys.key_camera_up))
    camera.pos.y -= delta_time_s * camera.speed_current;
  if (app.get_input().get_key_held(camera.keys.key_camera_down))
    camera.pos.y += delta_time_s * camera.speed_current;
};

}; // namespace camera

namespace bullet {

GameObject2D
create_bullet()
{
  GameObject2D game_object;
  game_object.sprite = bullet_sprite;
  game_object.tex_slot = tex_unit_kenny_nl;
  game_object.collision_layer = CollisionLayer::Bullet;
  game_object.name = "bullet";
  game_object.pos = { 0.0f, 0.0f };
  game_object.angle_radians = 0.0;
  game_object.size = { 10.0f, 10.0f };
  game_object.velocity = { 0.0f, 0.0f };
  game_object.speed_default = 200.0f;
  game_object.speed_current = game_object.speed_default;
  game_object.time_alive_left = 6.0f;
  return game_object;
};

static void
update_game_logic(GameObject2D& obj, float delta_time_s)
{
  // pos
  float x = glm::sin(obj.angle_radians) * obj.velocity.x;
  float y = -glm::cos(obj.angle_radians) * obj.velocity.y;
  obj.pos.x += x * delta_time_s;
  obj.pos.y += y * delta_time_s;

  // lifecycle
  obj.time_alive_left -= delta_time_s;
  if (obj.time_alive_left <= 0.0f) {
    obj.flag_for_delete = true;
  }
}

} // namespace: bullet

namespace player {

GameObject2D
create_player(const PlayerKeys& keys)
{
  GameObject2D game_object;
  game_object.sprite = player_sprite;
  game_object.tex_slot = tex_unit_kenny_nl;
  game_object.collision_layer = CollisionLayer::Player;
  game_object.name = "player";
  game_object.pos = { screen_width / 2.0f, screen_height / 2.0f };
  game_object.angle_radians = 0.0;
  game_object.size = { 1.0f * 768.0f / 48.0f, 1.0f * 362.0f / 22.0f };
  game_object.velocity = { 0.0f, 0.0f };
  game_object.velocity_boost_modifier = 2.0f;
  game_object.speed_default = 50.0f;
  game_object.speed_current = game_object.speed_default;
  game_object.invulnerable = false;
  game_object.hits_able_to_be_taken = 10;
  game_object.keys = keys;
  return game_object;
};

static void
update_input(GameObject2D& obj, Application& app)
{
  obj.l_analogue_x = 0.0f;
  obj.l_analogue_y = 0.0f;
  obj.shoot_pressed = false;
  obj.boost_pressed = false;
  obj.pause_pressed = false;

  // Keymaps: keyboard
  if (obj.use_keyboard) {
    if (app.get_input().get_key_held(obj.keys.w)) {
      obj.l_analogue_y = -1.0f;
    } else if (app.get_input().get_key_held(obj.keys.s)) {
      obj.l_analogue_y = 1.0f;
    } else {
      obj.l_analogue_y = 0.0f;
    }
  }

  //   if (app.get_input().get_key_held(key_move_left)) {
  //     l_analogue_x = -1.0f;
  //   } else if (app.get_input().get_key_held(key_move_right)) {
  //     l_analogue_x = 1.0f;
  //   } else {
  //     l_analogue_x = 0.0f;
  //   }

  //   shoot_pressed = app.get_input().get_mouse_lmb_held();
  //   boost_pressed = app.get_input().get_key_held(key_boost);
  //   pause_pressed = app.get_input().get_key_down(SDL_SCANCODE_P);

  //   glm::vec2 player_world_space_pos = gameobject_in_worldspace(camera, player);
  //   float mouse_angle_around_player = atan2(app.get_input().get_mouse_pos().y - player_world_space_pos.y,
  //                                           app.get_input().get_mouse_pos().x - player_world_space_pos.x);
  //   mouse_angle_around_player += HALF_PI;
  //   look_angle = mouse_angle_around_player;

  // }
  // // Keymaps: Controller
  // else {
  //   l_analogue_x = app.get_input().get_axis_dir(controller, SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_LEFTX);
  //   l_analogue_y = app.get_input().get_axis_dir(controller, SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_LEFTY);
  //   r_analogue_x = app.get_input().get_axis_dir(controller, SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_RIGHTX);
  //   r_analogue_y = app.get_input().get_axis_dir(controller, SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_RIGHTY);
  //   shoot_pressed =
  //     app.get_input().get_axis_held(controller, SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_TRIGGERRIGHT);
  //   boost_pressed =
  //     app.get_input().get_axis_held(controller, SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_TRIGGERLEFT);
  //   pause_pressed =
  //     app.get_input().get_button_held(controller, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_START);

  //   look_angle = atan2(r_analogue_y, r_analogue_x);
  //   look_angle += HALF_PI;
  // }
}

static void
update_game_logic(GameObject2D& obj, float delta_time_s)
{
  // process input
  obj.velocity.x = obj.l_analogue_x;
  obj.velocity.y = obj.l_analogue_y;
  obj.velocity *= obj.speed_current;

  // Ability: Boost
  if (obj.boost_pressed)
    obj.velocity *= obj.velocity_boost_modifier;

  float look_angle = 0.0f;
  // look in mouse direction
  bool look_at_mouse = true;
  if (look_at_mouse) {
    obj.angle_radians = look_angle;
  }
  // look in velocity direction
  else {
    if (glm::length2(obj.velocity) > 0) {
      glm::vec2 up_axis = glm::vec2(0.0, -1.0);
      float unsigned_angle = glm::angle(up_axis, obj.velocity);
      float sign = (up_axis.x * obj.velocity.y - up_axis.y * obj.velocity.x) >= 0.0f ? 1.0f : -1.0f;
      float signed_angle = unsigned_angle * sign;

      obj.angle_radians = signed_angle;
    }
  }

  // Ability: Triple Burst Shoot
  if (obj.shoot_pressed)
    bullets_to_fire_after_releasing_mouse_left = bullets_to_fire_after_releasing_mouse;

  if (bullet_seconds_between_spawning_left > 0.0f)
    bullet_seconds_between_spawning_left -= delta_time_s;

  if (bullet_seconds_between_spawning_left <= 0.0f) {
    bullet_seconds_between_spawning_left = bullet_seconds_between_spawning;
    bullets_to_fire_after_releasing_mouse_left -= 1;

    std::cout << "todo, shoot bullet" << std::endl;

    // GameObject2D bullet_copy = bullet::create_bullet();

    // glm::vec2 bullet_pos = obj.pos;
    // bullet_pos.x += obj.size.x / 2.0f - bullet_copy.size.x / 2.0f;
    // bullet_pos.y += obj.size.y / 2.0f - bullet_copy.size.y / 2.0f;

    // // override defaults
    // bullet_copy.pos = bullet_pos;
    // bullet_copy.angle_radians = look_angle;
    // bullet_copy.velocity.x = bullet_copy.speed_current;
    // bullet_copy.velocity.y = bullet_copy.speed_current;

    // entities_bullets.push_back(bullet_copy);
  }
}

}; // namespace: player

namespace enemy {

GameObject2D
create_enemy()
{
  GameObject2D game_object;
  game_object.sprite = wall_sprite;
  game_object.tex_slot = tex_unit_kenny_nl;
  game_object.collision_layer = CollisionLayer::Destroyable;
  game_object.name = "wall";
  game_object.angle_radians = 0.0;
  game_object.size = { 20.0f, 20.0f };
  game_object.hits_able_to_be_taken = 3;
  return game_object;
};

static void
update_game_logic(GameObject2D& obj, GameObject2D& player, float delta_time_s)
{
  //
  // simple chase ai
  //

  // rotate to player
  glm::vec2 dir = player.pos - obj.pos;
  dir = glm::normalize(dir);
  obj.angle_radians = atan2(dir.y, dir.x);
  obj.angle_radians += fightingengine::PI / 2.0f;

  // move to player
  obj.velocity = glm::vec2(enemy_default_speed);
  float x = glm::sin(obj.angle_radians) * obj.velocity.x;
  float y = -glm::cos(obj.angle_radians) * obj.velocity.y;
  obj.pos.x += x * delta_time_s;
  obj.pos.y += y * delta_time_s;
}

} // namespace: enemy

int
main()
{
  std::cout << "booting up..." << std::endl;
  const auto app_start = std::chrono::high_resolution_clock::now();

  RandomState rnd;
  Application app("2D Game", static_cast<int>(screen_width), static_cast<int>(screen_height));
  app.set_fps_limit(60.0f);
  Profiler profiler;
  Console console;

  glm::mat4 projection = glm::ortho(0.0f, screen_width, screen_height, 0.0f, -1.0f, 1.0f);

  // controllers
  // could improve: currently only checks for controller once at start of app
  bool use_keyboard = true;
  SDL_GameController* controller = NULL;
  for (int i = 0; i < SDL_NumJoysticks(); ++i) {
    if (SDL_IsGameController(i)) {
      controller = SDL_GameControllerOpen(i);
      if (controller) {
        use_keyboard = false;
        break;
      } else {
        fprintf(stderr, "Could not open gamecontroller %i: %s\n", i, SDL_GetError());
      }
    }
  }

  // textures

  std::vector<std::pair<int, std::string>> textures_to_load;
  textures_to_load.emplace_back(tex_unit_kenny_nl,
                                "assets/textures/kennynl_1bit_pack/Tilesheet/monochrome_transparent_packed.png");
  load_textures_threaded(textures_to_load, app_start);

  // TODO sound

  // Rendering

  RenderCommand::init();
  RenderCommand::set_viewport(0, 0, static_cast<uint32_t>(screen_width), static_cast<uint32_t>(screen_height));
  RenderCommand::set_depth_testing(false); // disable depth testing for 2d
  sprite_renderer::init();

  Shader colour_shader = Shader("2d_basic.vert", "2d_colour.frag");
  colour_shader.bind();
  colour_shader.set_vec4("colour", chosen_colour_1);

  Shader instanced_quad_shader = Shader("2d_instanced.vert", "2d_instanced.frag");
  instanced_quad_shader.bind();
  instanced_quad_shader.set_mat4("projection", projection);
  instanced_quad_shader.set_int("tex", tex_unit_kenny_nl);

  // game

  // { // Random
  //   GameObject2D logo_entity;
  //   logo_entity.sprite = logo_sprite;
  //   logo_entity.tex_slot = tex_unit_kenny_nl;
  //   logo_entity.name = "logo";
  //   logo_entity.pos = { screen_width / 2.0f, screen_height / 2.0f };
  //   logo_entity.size = { 4.0f * 768.0f / 48.0f, 4.0f * 362.0f / 22.0f };

  //   GameObject2D tex_obj;
  //   tex_obj.tex_slot = tex_unit_kenny_nl;
  //   tex_obj.name = "texture_sheet";
  //   tex_obj.pos = { 0.0f, 20.0f };
  //   tex_obj.size = { 768.0f, 352.0f };

  //   GameObject2D tex_background;
  //   tex_background.sprite = sprite::type::SQUARE;
  //   tex_background.tex_slot = tex_unit_kenny_nl;
  //   tex_background.name = "textured_background";
  //   tex_background.size = { screen_width, screen_height };
  // }

  //
  // entities
  //

  GameObject2D camera = camera::create_camera();

  PlayerKeys player0_keys;
  GameObject2D player0 = player::create_player(player0_keys);
  player0.use_keyboard = true;

  PlayerKeys player1_keys;
  GameObject2D player1 = player::create_player(player1_keys);

  std::vector<GameObject2D> entities_walls;
  std::vector<GameObject2D> entities_bullets;

  float objects_destroyed = 0;

  std::cout << " a game object is: " << sizeof(GameObject2D) << " bytes" << std::endl;
  log_time_since("(INFO) End Setup ", app_start);

  // ---- App ----

  while (app.is_running()) {

    profiler.new_frame();
    profiler.begin(Profiler::Stage::UpdateLoop);

    app.frame_begin(); // get input events
    float delta_time_s = app.get_delta_time();

    profiler.begin(Profiler::Stage::Physics);
    {
      if (state == GameState::GAME_ACTIVE || (state == GameState::GAME_PAUSED && advance_one_frame)) {

        // hopefully no copy of GameObject2D
        std::vector<std::reference_wrapper<GameObject2D>> collidable;
        collidable.insert(collidable.end(), entities_walls.begin(), entities_walls.end());
        collidable.insert(collidable.end(), entities_bullets.begin(), entities_bullets.end());
        collidable.push_back(player0);
        collidable.push_back(player1);

        std::map<uint64_t, Collision2D> collisions;

        // broadphase: detect collisions can actually happen and discard collisions which can't.
        // sort and prune algorithm. note: suffers from large worlds with inactive objects.
        // this issue can be solved by using multiple smaller SAP's which form a grid.
        // note: i've adjusted this algortihm to do 2-axis SAP.

        // Do broad-phase check.

        // Sort entities by X-axis
        std::vector<std::reference_wrapper<GameObject2D>> sorted_collidable_x = collidable;
        std::sort(sorted_collidable_x.begin(),
                  sorted_collidable_x.end(),
                  [](std::reference_wrapper<GameObject2D> a, std::reference_wrapper<GameObject2D> b) {
                    return a.get().pos.x < b.get().pos.x;
                  });
        // SAP x-axis
        std::vector<std::pair<int, int>> potential_collisions_x;
        generate_broadphase_collisions(sorted_collidable_x, COLLISION_AXIS::X, collisions);

        // Sort entities by Y-axis
        std::vector<std::reference_wrapper<GameObject2D>> sorted_collidable_y = collidable;
        std::sort(sorted_collidable_y.begin(),
                  sorted_collidable_y.end(),
                  [](std::reference_wrapper<GameObject2D> a, std::reference_wrapper<GameObject2D> b) {
                    return a.get().pos.y < b.get().pos.y;
                  });
        // SAP y-axis
        std::vector<std::pair<int, int>> potential_collisions_y;
        generate_broadphase_collisions(sorted_collidable_y, COLLISION_AXIS::Y, collisions);

        // use broad-phase results....
        std::map<uint64_t, Collision2D> filtered_collisions;
        for (auto& coll : collisions) {
          Collision2D c = coll.second;
          if (c.collision_x && c.collision_y) {
            filtered_collisions[coll.first] = coll.second;
          }
        }

        // TODO: narrow-phase; per-model collision? convex shapes?

        // game's response

        for (auto& c : filtered_collisions) {

          uint32_t id_0 = c.second.ent_id_0;
          uint32_t id_1 = c.second.ent_id_1;

          // Iterate over every object... likely to get slow pretty quick.
          for (int i = 0; i < entities_walls.size(); i++) {
            GameObject2D& go = entities_walls[i];
            if (id_0 == go.id || id_1 == go.id) {

              go.hits_taken += 1;
              if (go.hits_taken >= go.hits_able_to_be_taken) {
                // what to do if a wall collided? remove it
                entities_walls.erase(entities_walls.begin() + i);
                objects_destroyed += 1;

                // other object was player?
                if (id_0 == player0.id || id_1 == player0.id) {
                  player0.hits_taken += 1;
                  std::cout << "player0 hit taken: " << player0.id << std::endl;
                }
                if (id_0 == player1.id || id_1 == player1.id) {
                  player1.hits_taken += 1;
                  std::cout << "player1 hit taken: " << player1.id << std::endl;
                }
              }
            }
          }
          for (int i = 0; i < entities_bullets.size(); i++) {
            GameObject2D& go = entities_bullets[i];
            if (id_0 == go.id || id_1 == go.id) {
              // what to do if a bullet collided? remove it
              entities_bullets.erase(entities_bullets.begin() + i);
            }
          }
        }
      }
    }
    profiler.end(Profiler::Stage::Physics);
    profiler.begin(Profiler::Stage::SdlInput);
    {
#ifdef _DEBUG

      // Debug: Advance one frame
      if (app.get_input().get_key_down(SDL_SCANCODE_RSHIFT)) {
        advance_one_frame = true;
      }
      // Debug: Advance frames
      if (app.get_input().get_key_held(SDL_SCANCODE_F10)) {
        advance_one_frame = true;
      }
      // Debug: Force game over
      if (app.get_input().get_key_down(SDL_SCANCODE_F11)) {
        state = GameState::GAME_OVER_SCREEN;
      }

      // Debug: Reset player pos
      if (app.get_input().get_key_held(SDL_SCANCODE_O)) {
        player0.pos = glm::vec2(0.0f, 0.0f);
        player1.pos = glm::vec2(100.0f, 0.0f);
      }

      // Settings: Exit App
      if (app.get_input().get_key_down(key_quit))
        app.shutdown();

      // Settings: Toggle Console
      if (app.get_input().get_key_down(key_console))
        show_console = !show_console;

      // Settings: Fullscreen
      if (app.get_input().get_key_down(key_fullscreen)) {
        app.get_window().toggle_fullscreen(); // SDL2 window toggle
        glm::ivec2 screen_size = app.get_window().get_size();
        RenderCommand::set_viewport(0, 0, screen_size.x, screen_size.y);
        screen_width = static_cast<float>(screen_size.x);
        screen_height = static_cast<float>(screen_size.y);
        projection = glm::ortho(0.0f, screen_width, screen_height, 0.0f, -1.0f, 1.0f);
        instanced_quad_shader.bind();
        instanced_quad_shader.set_mat4("projection", projection);
      }

      // Shader hot reloading
      // if (app.get_input().get_key_down(SDL_SCANCODE_R)) {
      //   reload_shader_program(&fun_shader.ID, "2d_texture.vert", "effects/posterized_water.frag");
      //   fun_shader.bind();
      //   fun_shader.set_mat4("projection", projection);
      //   fun_shader.set_int("tex", tex_unit_kenny_nl);
      // }
#endif // _DEBUG
    }
    profiler.end(Profiler::Stage::SdlInput);
    profiler.begin(Profiler::Stage::GameTick);
    {
      player::update_input(player0, app);
      player::update_input(player1, app);

      if (state == GameState::GAME_ACTIVE) {

        // pause game
        if (player0.pause_pressed || player1.pause_pressed) {
          state = state == GameState::GAME_PAUSED ? GameState::GAME_ACTIVE : GameState::GAME_PAUSED;
        }

        player::update_game_logic(player0, delta_time_s);
        player::update_game_logic(player1, delta_time_s);

        bool player0_alive = player0.invulnerable && player0.hits_taken > player0.hits_able_to_be_taken;
        if (!player0_alive) {
          state = GameState::GAME_OVER_SCREEN;
        }

        gameobject::update_position(player0, delta_time_s);
        gameobject::update_position(player1, delta_time_s);
        // gameobject::update_position(bullets);
        // gameobject::update_position(enemies);

        // Gameplay: Spawn Enemies
        wall_seconds_between_spawning_left -= delta_time_s;
        if (wall_seconds_between_spawning_left <= 0.0f) {
          wall_seconds_between_spawning_left = wall_seconds_between_spawning;

          // glm::ivec2 mouse_pos = app.get_input().get_mouse_pos();
          // printf("(game) mmb clicked %i %i \n", mouse_pos.x, mouse_pos.y);
          // glm::vec2 world_pos = glm::vec2(mouse_pos) + camera.pos;

          glm::vec2 rnd_pos =
            glm::vec2(rand_det_s(rnd.rng, 0.0f, 1.0f) * screen_width, rand_det_s(rnd.rng, 0.0f, 1.0f) * screen_height);
          glm::vec2 world_pos = rnd_pos + camera.pos;

          // todo: spawn wall
          // GameObject2D wall_copy = wall;
          // wall_copy.pos = world_pos; // override defaults
          // entities_walls.push_back(wall_copy);
        }

        // todo: manage lifecycle and delete expired objects
      }
    }
    profiler.end(Profiler::Stage::GameTick);
    profiler.begin(Profiler::Stage::Render);
    {

      glm::ivec2 screen_wh = glm::ivec2(screen_width, screen_height);
      RenderCommand::set_clear_colour(background_colour);
      RenderCommand::clear();
      sprite_renderer::reset_stats();
      sprite_renderer::begin_batch();
      instanced_quad_shader.bind();

      if (state == GameState::GAME_ACTIVE || state == GameState::GAME_PAUSED) {
        
        std::vector<std::reference_wrapper<GameObject2D>> renderables;
        renderables.insert(renderables.end(), entities_walls.begin(), entities_walls.end());
        renderables.insert(renderables.end(), entities_bullets.begin(), entities_bullets.end());
        renderables.push_back(player0);
        renderables.push_back(player1);

        for (std::reference_wrapper<GameObject2D> obj : renderables) {
          
          GameObject2D& o = obj.get();

          sprite_renderer::draw_sprite_debug(
            camera, screen_wh, instanced_quad_shader, o, colour_shader, debug_line_colour);
        }
      }

      sprite_renderer::end_batch();
      sprite_renderer::flush(instanced_quad_shader);
    }
    profiler.end(Profiler::Stage::Render);
    profiler.begin(Profiler::Stage::GuiLoop);
    {
      if (ImGui::BeginMainMenuBar()) {
        if (ImGui::MenuItem("Quit", "Esc"))
          app.shutdown();
        ImGui::SameLine(ImGui::GetWindowWidth() - 154.0f);
        ImGui::Text("%.2f FPS (%.2f ms)", ImGui::GetIO().Framerate, 1000.0f / ImGui::GetIO().Framerate);
        ImGui::EndMainMenuBar();
      }

      if (show_game_info) {
        ImGui::Begin("Game Info", NULL, ImGuiWindowFlags_NoFocusOnAppearing);
        {
          ImGui::Text("game running for: %f", app.seconds_since_launch);
          ImGui::Text("player0 pos %f %f", player0.pos.x, player0.pos.y);
          ImGui::Text("player0 vel x: %f y: %f", player0.velocity.x, player0.velocity.y);
          ImGui::Text("player0 angle %f", player0.angle_radians);
          ImGui::Text("player0 hp_max %i", player0.hits_able_to_be_taken);
          ImGui::Text("player0 hits taken %i", player0.hits_taken);
          ImGui::Text("player1 pos %f %f", player1.pos.x, player1.pos.y);
          ImGui::Text("player1 vel x: %f y: %f", player1.velocity.x, player1.velocity.y);
          ImGui::Text("player1 angle %f", player1.angle_radians);
          ImGui::Text("player1 hp_max %i", player1.hits_able_to_be_taken);
          ImGui::Text("player1 hits taken %i", player1.hits_taken);
          ImGui::Text("camera pos %f %f", camera.pos.x, camera.pos.y);
          ImGui::Text("mouse pos %f %f", app.get_input().get_mouse_pos().x, app.get_input().get_mouse_pos().y);
          ImGui::Separator();
          // ImGui::Text("highscore: %i", highscore);
          ImGui::Text("Walls: %i", entities_walls.size());
          ImGui::Text("Bullets: %i", entities_bullets.size());
          ImGui::Text("Bullets to fire: %i", bullets_to_fire_after_releasing_mouse_left);
          ImGui::Text("(game) destroyed: %i", objects_destroyed);

          ImGui::Separator();
          ImGui::Text("controllers %i", SDL_NumJoysticks());
          ImGui::Separator();
          ImGui::Text("draw_calls: %i", sprite_renderer::get_draw_calls());
          ImGui::Text("quad_verts: %i", sprite_renderer::get_quad_count());
          // ImGui::Separator();
          // ImGui::Checkbox("Aces Tone Mapping", &aces_tone_mapping);
          ImGui::Separator();
          static float col_0[4] = { chosen_colour_0.x, chosen_colour_0.y, chosen_colour_0.z, chosen_colour_0.w };
          ImGui::ColorEdit4("col 0", col_0);
          chosen_colour_0 = glm::vec4(col_0[0], col_0[1], col_0[2], col_0[3]);
          static float col_1[4] = { chosen_colour_1.x, chosen_colour_1.y, chosen_colour_1.z, chosen_colour_1.w };
          ImGui::ColorEdit4("col 1", col_1);
          chosen_colour_1 = glm::vec4(col_1[0], col_1[1], col_1[2], col_1[3]);
          static float col_2[4] = { chosen_colour_2.x, chosen_colour_2.y, chosen_colour_2.z, chosen_colour_2.w };
          ImGui::ColorEdit4("col 2", col_2);
          chosen_colour_2 = glm::vec4(col_2[0], col_2[1], col_2[2], col_2[3]);
          static float col_3[4] = { chosen_colour_3.x, chosen_colour_3.y, chosen_colour_3.z, chosen_colour_3.w };
          ImGui::ColorEdit4("col 3", col_3);
          chosen_colour_3 = glm::vec4(col_3[0], col_3[1], col_3[2], col_3[3]);
        }
        ImGui::End();
      }

      if (show_console)
        console.Draw("Console", &show_console);
      if (show_profiler)
        profiler_panel::draw(profiler, delta_time_s);
      if (show_demo_window)
        ImGui::ShowDemoWindow(&show_demo_window);
    }
    profiler.end(Profiler::Stage::GuiLoop);
    profiler.begin(Profiler::Stage::FrameEnd);
    {
      advance_one_frame = false;
      app.frame_end(delta_time_s);
    }
    profiler.end(Profiler::Stage::FrameEnd);
    profiler.end(Profiler::Stage::UpdateLoop);
  }
}
