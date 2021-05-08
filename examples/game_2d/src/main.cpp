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

// fightingengine headers
#include "engine/application.hpp"
#include "engine/maths_core.hpp"
#include "engine/opengl/render_command.hpp"
#include "engine/ui/profiler_panel.hpp"
#include "engine/util.hpp"
using namespace fightingengine;

// game headers
#include "console.hpp"
#include "sprite_renderer.hpp"
#include "spritemap.hpp"
using namespace game2d;

enum class GameState
{
  GAME_SPLASH_SCREEN,
  GAME_OVER_SCREEN,
  GAME_ACTIVE,
  GAME_PAUSED
};
GameState state = GameState::GAME_SPLASH_SCREEN;
const float time_on_splash_screen = 0.0f;
float time_on_splash_screen_left = time_on_splash_screen;
const float time_on_game_over_screen = 3.0f;
float time_on_game_over_screen_left = time_on_game_over_screen;

// default colour palette
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
// colours
glm::vec4 background_colour = chosen_colour_0;
glm::vec4 player_colour = chosen_colour_1;
glm::vec4 bullet_colour = chosen_colour_2;
glm::vec4 wall_colour = chosen_colour_3;
glm::vec4 line_debug_colour = chosen_colour_3;
// sprites
sprite::type logo_sprite = sprite::type::WALL_BIG;
sprite::type player_sprite = sprite::type::TREE_1;
sprite::type bullet_sprite = sprite::type::TREE_1;
sprite::type wall_sprite = sprite::type::WALL_BIG;
// textures
const int tex_unit_kenny_nl = 0;
// app
float screen_width = 1366.0f;
float screen_height = 768.0f;
bool show_game_info = true;
bool show_profiler = true;
bool show_console = false;
bool show_demo_window = false;
bool advance_one_frame = false;
// mouse
float mouse_angle_around_player = 0.0f;
// key bindings: application
SDL_Scancode key_quit = SDL_SCANCODE_ESCAPE;
SDL_Scancode key_console = SDL_SCANCODE_F12;
SDL_Scancode key_fullscreen = SDL_SCANCODE_F;
// key bindings: game
SDL_Scancode key_camera_up = SDL_SCANCODE_UP;
SDL_Scancode key_camera_down = SDL_SCANCODE_DOWN;
SDL_Scancode key_camera_left = SDL_SCANCODE_LEFT;
SDL_Scancode key_camera_right = SDL_SCANCODE_RIGHT;
SDL_Scancode key_move_up = SDL_SCANCODE_W;
SDL_Scancode key_move_down = SDL_SCANCODE_S;
SDL_Scancode key_move_left = SDL_SCANCODE_A;
SDL_Scancode key_move_right = SDL_SCANCODE_D;
SDL_Scancode key_boost = SDL_SCANCODE_LSHIFT;
SDL_Scancode key_camera_follow_player = SDL_SCANCODE_Q;
// controller bindings

// simple aabb collision
bool
collides(GameObject2D& one, GameObject2D& two)
{
  // collision x-axis?
  bool collisionX = one.pos.x + one.size.x >= two.pos.x && two.pos.x + two.size.x >= one.pos.x;
  // collision y-axis?
  bool collisionY = one.pos.y + one.size.y >= two.pos.y && two.pos.y + two.size.y >= one.pos.y;
  // collision only if on both axes
  return collisionX && collisionY;
}

// game collision matrix
bool
collides_matrix(CollisionLayer& y_l1, CollisionLayer& x_l2)
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
    true, // default_detroyable 0_3

    true,  // player_default_1_0
    true,  // player_player_1_1
    false, // player_bullet_1_2
    true,  // player_destroyable_1_3

    true,  // bullet_default_2_0
    false, // bullet_player_2_1
    false, // bullet_bullet_2_2
    true,  // bullet_destroyable_2_3

    true,  // destroyable_default
    true,  // destroyable_player
    true,  // destroyable_bullet
    false, // destroyable_destroyable
  };

  return collision_matrix[val];
}

void
generate_collisions(std::vector<GameObject2D>& collidable, std::vector<Collision2D>& collisions)
{
  std::vector<GameObject2D>::iterator it_1 = collidable.begin();
  while (it_1 != collidable.end()) {
    std::vector<GameObject2D>::iterator it_2 = collidable.begin();
    while (it_2 != collidable.end()) {
      GameObject2D& obj_1 = *it_1;
      GameObject2D& obj_2 = *it_2;
      if (obj_1.id == obj_2.id) {
        ++it_2;
        continue;
      }
      bool collision_config = collides_matrix(obj_1.collision_layer, obj_2.collision_layer);
      if (collision_config && collides(obj_1, obj_2)) {
        Collision2D col;
        col.obj_id_1 = obj_1.id;
        col.obj_id_2 = obj_2.id;
        collisions.push_back(col);
        // todo: turn this in to a set, so that collision a, b is the same as b, a
      }
      ++it_2;
    }
    ++it_1;
  }
};

void
resolve_collisions(std::vector<Collision2D>& collisions,
                   int& objects_collected,
                   std::vector<GameObject2D>& walls,
                   std::vector<GameObject2D>& bullets,
                   GameObject2D& player)
{
  for (Collision2D c : collisions) {
    // Iterate over every object... likely to get slow pretty quick.
    for (int i = 0; i < walls.size(); i++) {
      GameObject2D& go = walls[i];
      if (c.obj_id_1 == go.id || c.obj_id_2 == go.id) {
        // what to do if a wall collided? remove it
        walls.erase(walls.begin() + i);
        objects_collected += 1;
        break;
      }
    }
    for (int i = 0; i < bullets.size(); i++) {
      GameObject2D& go = bullets[i];
      if (c.obj_id_1 == go.id || c.obj_id_2 == go.id) {
        // what to do if a bullet collided? remove it
        bullets.erase(bullets.begin() + i);
        break;
      }
    }
    // player collided
    // bug: collisions generate both ways, so this will increment twice.
    if (c.obj_id_1 == player.id || c.obj_id_2 == player.id) {
      if (!player.invulnerable) {
        player.hits_taken += 1;
      }
    }
  }
};

void
player_movement(Application& app, GameObject2D& player)
{
  if (app.get_input().get_key_held(key_move_left)) {
    player.velocity.x = -1.0f;
  } else if (app.get_input().get_key_held(key_move_right)) {
    player.velocity.x = 1.0f;
  } else {
    player.velocity.x = 0.0f;
  }

  if (app.get_input().get_key_held(key_move_up)) {
    player.velocity.y = -1.0f;
  } else if (app.get_input().get_key_held(key_move_down)) {
    player.velocity.y = 1.0f;
  } else {
    player.velocity.y = 0.0f;
  }
  player.velocity *= player.speed_current;

  // Ability: Boost
  if (app.get_input().get_key_held(key_boost))
    player.velocity *= player.velocity_boost_modifier;
};

void
player_rotation(Application& app, GameObject2D& player, GameObject2D& camera, float& mouse_angle_around_player)
{
  // look in mouse direction
  bool look_at_mouse = true;
  if (look_at_mouse) {
    glm::vec2 player_world_space_pos = player.pos - camera.pos;
    mouse_angle_around_player = atan2(app.get_input().get_mouse_pos().y - player_world_space_pos.y,
                                      app.get_input().get_mouse_pos().x - player_world_space_pos.x);
    mouse_angle_around_player += HALF_PI;

    player.angle_radians = mouse_angle_around_player;
  }
  // look in velocity direction
  else {
    if (glm::length2(player.velocity) > 0) {
      glm::vec2 up_axis = glm::vec2(0.0, -1.0);
      float unsigned_angle = glm::angle(up_axis, player.velocity);
      float sign = (up_axis.x * player.velocity.y - up_axis.y * player.velocity.x) >= 0.0f ? 1.0f : -1.0f;
      float signed_angle = unsigned_angle * sign;

      player.angle_radians = signed_angle;
    }
  }
};

// ai for chasing player
void
chase_player(GameObject2D& player, const float delta_time, std::vector<GameObject2D>& enemies)
{
  for (int i = 0; i < enemies.size(); ++i) {
    GameObject2D& e = enemies[i];

    // rotate to player
    glm::vec2 dir = player.pos - e.pos;
    dir = glm::normalize(dir);
    e.angle_radians = atan2(dir.y, dir.x);
    e.angle_radians += PI / 2.0f;

    // move to player
    const float monster_speed = 50.0f;
    e.velocity = glm::vec2(monster_speed);
    float x = glm::sin(e.angle_radians) * e.velocity.x;
    float y = -glm::cos(e.angle_radians) * e.velocity.y;
    e.pos.x += x * delta_time;
    e.pos.y += y * delta_time;
  }
}

void
reset_game(GameObject2D& camera,
           GameObject2D& player,
           int& objects_collected,
           std::vector<GameObject2D>& entities_bullets,
           std::vector<GameObject2D>& entities_walls,
           std::vector<Collision2D>& collisions)
{
  camera.pos = glm::vec2{ 0.0f, 0.0f };

  player.name = "player";
  player.pos = { screen_width / 2.0f, screen_height / 2.0f };
  player.angle_radians = 0.0;
  player.size = { 1.0f * 768.0f / 48.0f, 1.0f * 362.0f / 22.0f };
  player.colour = player_colour;
  player.velocity = { 0.0f, 0.0f };
  player.velocity_boost_modifier = 2.0f;
  player.speed_default = 50.0f;
  player.speed_current = player.speed_default;
  player.invulnerable = true;

  objects_collected = 0;

  entities_bullets.clear();
  entities_walls.clear();
  collisions.clear();
}

int
main()
{
  std::cout << "booting up..." << std::endl;
  const auto app_start = std::chrono::high_resolution_clock::now();

  RandomState rnd;
  Application app("2D Game", static_cast<int>(screen_width), static_cast<int>(screen_height));
  // app.set_fps_limit(60.0f);
  Profiler profiler;
  Console console;

  // controller info
  bool use_keyboard = true;
  int num_controllers_plugged_in = SDL_NumJoysticks();
  SDL_Joystick* controller_0 = NULL;
  if (num_controllers_plugged_in > 0) {
    use_keyboard = false;
    controller_0 = SDL_JoystickOpen(0);
    if (controller_0 == NULL) {
      printf("Warning: Unable to open game controller! SDL Error: %s\n", SDL_GetError());
    }
  }

  //
  // textures
  //
  std::vector<std::pair<int, std::string>> textures_to_load;
  textures_to_load.emplace_back(tex_unit_kenny_nl,
                                "assets/textures/kennynl_1bit_pack/Tilesheet/monochrome_transparent_packed.png");
  load_textures_threaded(textures_to_load, app_start);

  //
  // TODO sound
  //

  //
  // Rendering
  //
  RenderCommand::init();
  RenderCommand::set_clear_colour(background_colour);
  RenderCommand::set_viewport(0, 0, static_cast<uint32_t>(screen_width), static_cast<uint32_t>(screen_height));
  RenderCommand::set_depth_testing(false); // disable depth testing for 2d

  GameObject2D camera;
  glm::mat4 projection = glm::ortho(0.0f, screen_width, screen_height, 0.0f, -1.0f, 1.0f);

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

  Shader fun_shader = Shader("2d_texture.vert", "effects/posterized_water.frag");
  fun_shader.bind();
  fun_shader.set_mat4("projection", projection);
  fun_shader.set_int("tex", tex_unit_kenny_nl);
  fun_shader.set_float("time", app.seconds_since_launch);

  sprite::spritemap spritemap;
  auto& sprites = spritemap.get_locations();

  //
  // Game
  //
  GameObject2D logo_entity;
  logo_entity.sprite = logo_sprite;
  logo_entity.tex_slot = tex_unit_kenny_nl;
  logo_entity.name = "logo";
  logo_entity.pos = { screen_width / 2.0f, screen_height / 2.0f };
  logo_entity.angle_radians = 0.0;
  logo_entity.size = { 4.0f * 768.0f / 48.0f, 4.0f * 362.0f / 22.0f };
  logo_entity.colour = player_colour;
  logo_entity.velocity = { 0.0f, 0.0f };

  GameObject2D player;
  player.sprite = player_sprite;
  player.tex_slot = tex_unit_kenny_nl;
  player.collision_layer = CollisionLayer::Player;

  float bullet_seconds_between_spawning = 0.01f;
  float bullet_seconds_between_spawning_left = 0.0f;
  GameObject2D bullet;
  bullet.sprite = bullet_sprite;
  bullet.tex_slot = tex_unit_kenny_nl;
  bullet.collision_layer = CollisionLayer::Bullet;
  bullet.name = "bullet";
  bullet.pos = { 0.0f, 0.0f };
  bullet.angle_radians = 0.0;
  bullet.size = { 25.0f, 25.0f };
  bullet.colour = bullet_colour;
  bullet.velocity = { 0.0f, 0.0f };
  bullet.speed_default = 50.0f;
  bullet.speed_current = bullet.speed_default;
  bullet.time_alive_left = 10.0f;
  bullet.is_bullet = true;

  float wall_seconds_between_spawning = 1.0f;
  float wall_seconds_between_spawning_left = 0.0f;
  GameObject2D wall;
  wall.sprite = wall_sprite;
  wall.tex_slot = tex_unit_kenny_nl;
  wall.collision_layer = CollisionLayer::Destroyable;
  wall.name = "wall";
  wall.pos = { 0.0f, 0.0f };
  wall.angle_radians = 0.0;
  wall.size = { 20.0f, 20.0f };
  wall.colour = wall_colour;
  wall.velocity = { 0.0f, 0.0f };

  GameObject2D tex_obj;
  tex_obj.tex_slot = tex_unit_kenny_nl;
  tex_obj.name = "texture_sheet";
  tex_obj.pos = { 0.0f, 20.0f };
  tex_obj.angle_radians = 0.0f;
  tex_obj.size = { 768.0f, 352.0f };
  tex_obj.colour = { 1.0f, 1.0f, 1.0f, 1.0f };
  tex_obj.velocity = { 0.0f, 0.0f };

  GameObject2D tex_background;
  tex_background.sprite = sprite::type::SQUARE;
  tex_background.tex_slot = tex_unit_kenny_nl;
  tex_background.name = "textured_background";
  tex_background.pos = { 0.0f, 0.0f };
  tex_background.angle_radians = 0.0f;
  tex_background.size = { screen_width, screen_height };
  // tex_background.colour = { 1.0f, 0.0f, 0.0f, 0.1f };
  tex_background.velocity = { 0.0f, 0.0f };

  int objects_collected = 0;
  std::vector<GameObject2D> entities_bullets;
  std::vector<GameObject2D> entities_walls;
  std::vector<Collision2D> collisions;
  reset_game(camera, player, objects_collected, entities_bullets, entities_walls, collisions);

  log_time_since("(INFO) End Setup ", app_start);

  // ---- App ----

  while (app.is_running()) {

    profiler.new_frame();
    profiler.begin(Profiler::Stage::UpdateLoop);
    profiler.begin(Profiler::Stage::SdlInput);

    app.frame_begin(); // input events
    float delta_time_s = app.get_delta_time();

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
      sprite_shader.bind();
      sprite_shader.set_mat4("projection", projection);
      tex_shader.bind();
      tex_shader.set_mat4("projection", projection);
      colour_shader.bind();
      colour_shader.set_mat4("projection", projection);
    }

    // Shader hot reloading
    // if (app.get_input().get_key_down(SDL_SCANCODE_R)) {
    //   reload_shader_program(&fun_shader.ID, "2d_texture.vert", "effects/posterized_water.frag");
    //   fun_shader.bind();
    //   fun_shader.set_mat4("projection", projection);
    //   fun_shader.set_int("tex", tex_unit_kenny_nl);
    // }

    // Game: Pause
    if (app.get_input().get_key_down(SDL_SCANCODE_P)) {
      state = state == GameState::GAME_PAUSED ? GameState::GAME_ACTIVE : GameState::GAME_PAUSED;
    }

    // Game Thing: Reset player pos
    if (app.get_input().get_key_held(SDL_SCANCODE_O)) {
      player.pos = glm::vec2(0.0f, 0.0f);
    }

#ifdef _DEBUG

    // Debug: Advance one frame
    if (app.get_input().get_key_down(SDL_SCANCODE_RSHIFT)) {
      advance_one_frame = true;
    }
    // Debug: Advance frames
    if (app.get_input().get_key_held(SDL_SCANCODE_RCTRL)) {
      advance_one_frame = true;
    }
    // Debug: Force game over
    if (app.get_input().get_key_down(SDL_SCANCODE_F11)) {
      state = GameState::GAME_OVER_SCREEN;
    }

#endif

    profiler.end(Profiler::Stage::SdlInput);
    profiler.begin(Profiler::Stage::GameTick);
    {
      if (state == GameState::GAME_ACTIVE || (state == GameState::GAME_PAUSED && advance_one_frame)) {
        advance_one_frame = false;

        player_movement(app, player);
        player_rotation(app, player, camera, mouse_angle_around_player);

        //
        // Ability: Shoot
        //
        bullet_seconds_between_spawning_left -= delta_time_s;
        if (bullet_seconds_between_spawning_left <= 0.0f) {
          bullet_seconds_between_spawning_left = bullet_seconds_between_spawning;

          GameObject2D bullet_copy;
          // defaults
          bullet_copy.name = bullet.name;
          bullet_copy.size = bullet.size;
          bullet_copy.colour = bullet.colour;
          bullet_copy.sprite = bullet.sprite;
          bullet_copy.tex_slot = bullet.tex_slot;
          bullet_copy.collision_layer = bullet.collision_layer;
          bullet_copy.time_alive_left = bullet.time_alive_left;
          // override defaults
          bullet_copy.pos = player.pos;
          bullet_copy.angle_radians = mouse_angle_around_player;
          bullet_copy.velocity.x = bullet.speed_current;
          bullet_copy.velocity.y = bullet.speed_current;

          entities_bullets.push_back(bullet_copy);
        }

        //
        // Collision Detection
        //

        collisions.clear();

        // todo: don't take copy of objects here...!
        std::vector<GameObject2D> collidable;
        collidable.insert(collidable.end(), entities_walls.begin(), entities_walls.end());
        collidable.insert(collidable.end(), entities_bullets.begin(), entities_bullets.end());
        collidable.push_back(player);

        // todo broadphase: sort and sweep algorithm

        // narrow phase
        generate_collisions(collidable, collisions);
        // game's response
        resolve_collisions(collisions, objects_collected, entities_walls, entities_bullets, player);

        if (player.hits_taken > player.hits_able_to_be_taken) {
          state = GameState::GAME_OVER_SCREEN;
        }

        //
        // Update objects
        //

        // update: camera
        if (app.get_input().get_key_held(key_camera_left))
          camera.pos.x -= delta_time_s * camera.speed_current;
        if (app.get_input().get_key_held(key_camera_right))
          camera.pos.x += delta_time_s * camera.speed_current;
        if (app.get_input().get_key_held(key_camera_up))
          camera.pos.y -= delta_time_s * camera.speed_current;
        if (app.get_input().get_key_held(key_camera_down))
          camera.pos.y += delta_time_s * camera.speed_current;

        // update: player
        player.pos += player.velocity * delta_time_s;

        // update: bullets
        std::vector<GameObject2D>::iterator it_1 = entities_bullets.begin();
        while (it_1 != entities_bullets.end()) {
          GameObject2D& obj = (*it_1);
          // pos
          float x = glm::sin(obj.angle_radians) * obj.velocity.x;
          float y = -glm::cos(obj.angle_radians) * obj.velocity.y;
          obj.pos.x += x * delta_time_s;
          obj.pos.y += y * delta_time_s;
          // lifecycle
          obj.time_alive_left -= delta_time_s;
          if (obj.time_alive_left <= 0.0f) {
            it_1 = entities_bullets.erase(it_1);
          } else {
            ++it_1;
          }
        }

        // update: camera
        if (app.get_input().get_key_held(key_camera_follow_player)) {
          // pos
          camera.pos =
            glm::vec2(player.pos.x - screen_width / 2.0f, player.pos.y - screen_height / 2.0f) * delta_time_s;
        }

        //
        // AI: move to player
        //
        chase_player(player, delta_time_s, entities_walls);

        //
        // Gameplay: Spawn Enemies
        //
        wall_seconds_between_spawning_left -= delta_time_s;
        if (wall_seconds_between_spawning_left <= 0.0f) {
          wall_seconds_between_spawning_left = wall_seconds_between_spawning;

          // glm::ivec2 mouse_pos = app.get_input().get_mouse_pos();
          // printf("(game) mmb clicked %i %i \n", mouse_pos.x, mouse_pos.y);
          // glm::vec2 world_pos = glm::vec2(mouse_pos) + camera.pos;

          glm::vec2 rnd_pos =
            glm::vec2(rand_det_s(rnd.rng, 0.0f, 1.0f) * screen_width, rand_det_s(rnd.rng, 0.0f, 1.0f) * screen_height);
          glm::vec2 world_pos = rnd_pos + camera.pos;

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

          entities_walls.push_back(wall_copy);
        }
      }
    }
    profiler.end(Profiler::Stage::GameTick);
    profiler.begin(Profiler::Stage::Render);
    //
    // rendering
    //
    {
      glm::ivec2 screen_wh = glm::ivec2(screen_width, screen_height);

      if (state == GameState::GAME_SPLASH_SCREEN) {

        RenderCommand::set_clear_colour(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
        RenderCommand::clear();
        glm::ivec2 screen_wh = glm::ivec2(screen_width, screen_height);

        // draw logo
        glm::ivec2 obj = spritemap.get_sprite_offset(logo_entity.sprite);
        sprite_shader.bind();
        sprite_shader.set_int("desired_x", obj.x);
        sprite_shader.set_int("desired_y", obj.y);
        sprite_renderer::draw_sprite_debug(
          camera, screen_wh, sprite_shader, logo_entity, colour_shader, line_debug_colour);

        // when done
        time_on_splash_screen_left -= delta_time_s;
        if (time_on_splash_screen_left <= 0) {
          state = GameState::GAME_ACTIVE;
          time_on_splash_screen_left = time_on_splash_screen;
        }

      } else if (state == GameState::GAME_ACTIVE || state == GameState::GAME_PAUSED) {
        RenderCommand::set_clear_colour(background_colour);
        RenderCommand::clear();

        // draw: white square with fun shader
        // fun_shader.bind();
        // fun_shader.set_float("time", app.seconds_since_launch);
        // sprite_renderer::draw_sprite_debug(
        //   camera, screen_wh, fun_shader, tex_background, colour_shader, line_debug_colour);

        // draw: texture object
        // tex_shader.bind();
        // sprite_renderer::draw_sprite_debug(camera, screen_wh, tex_shader, tex_obj, colour_shader,
        // line_debug_colour); draw: walls
        glm::ivec2 obj;
        for (GameObject2D& object : entities_bullets) {
          obj = spritemap.get_sprite_offset(object.sprite);
          sprite_shader.bind();
          sprite_shader.set_int("desired_x", obj.x);
          sprite_shader.set_int("desired_y", obj.y);
          sprite_renderer::draw_sprite_debug(
            camera, screen_wh, sprite_shader, object, colour_shader, line_debug_colour);
        }
        // draw: bullets
        for (GameObject2D& object : entities_walls) {
          obj = spritemap.get_sprite_offset(object.sprite);
          sprite_shader.bind();
          sprite_shader.set_int("desired_x", obj.x);
          sprite_shader.set_int("desired_y", obj.y);
          sprite_renderer::draw_sprite_debug(
            camera, screen_wh, sprite_shader, object, colour_shader, line_debug_colour);
        }
        // draw: player
        obj = spritemap.get_sprite_offset(player.sprite);
        sprite_shader.bind();
        sprite_shader.set_int("desired_x", obj.x);
        sprite_shader.set_int("desired_y", obj.y);
        sprite_renderer::draw_sprite_debug(camera, screen_wh, sprite_shader, player, colour_shader, line_debug_colour);
      } else if (state == GameState::GAME_OVER_SCREEN) {
        RenderCommand::set_clear_colour(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
        RenderCommand::clear();
        glm::ivec2 screen_wh = glm::ivec2(screen_width, screen_height);

        // draw logo
        glm::ivec2 obj = spritemap.get_sprite_offset(player.sprite);
        sprite_shader.bind();
        sprite_shader.set_int("desired_x", obj.x);
        sprite_shader.set_int("desired_y", obj.y);
        sprite_renderer::draw_sprite_debug(
          camera, screen_wh, sprite_shader, logo_entity, colour_shader, line_debug_colour);

        // when done
        time_on_game_over_screen_left -= delta_time_s;
        if (time_on_game_over_screen_left <= 0) {
          reset_game(camera, player, objects_collected, entities_bullets, entities_walls, collisions);
          time_on_game_over_screen_left = time_on_game_over_screen;
          state = GameState::GAME_ACTIVE;
        }
      }
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

      if (show_game_info) {
        ImGui::Begin("Game Info", NULL, ImGuiWindowFlags_NoFocusOnAppearing);
        {
          ImGui::Text("game running for: %f", app.seconds_since_launch);
          ImGui::Text("player pos %f %f", player.pos.x, player.pos.y);
          ImGui::Text("player vel x: %f y: %f", player.velocity.x, player.velocity.y);
          ImGui::Text("player angle %f", player.angle_radians);
          ImGui::Text("camera pos %f %f", camera.pos.x, camera.pos.y);
          ImGui::Text("mouse pos %f %f", app.get_input().get_mouse_pos().x, app.get_input().get_mouse_pos().y);
          ImGui::Text("mouse angle around player %f", mouse_angle_around_player);
          ImGui::Separator();
          ImGui::Text("Walls: %i", entities_walls.size());
          ImGui::Text("Bullets: %i", entities_bullets.size());
          ImGui::Text("Collisions: %i", collisions.size());
          ImGui::Text("(game) collected: %i", objects_collected);
          ImGui::Text("(game) hp_max %i", player.hits_able_to_be_taken);
          ImGui::Text("(game) hp_remaining %i", player.hits_taken);
          ImGui::Separator();
          ImGui::Text("controllers %i", SDL_NumJoysticks());
          ImGui::Text("use keyboard %d", use_keyboard);
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