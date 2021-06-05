//
// A 2D thing.
//

// c++ lib headers
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

// other library headers
#include <SDL2/SDL_syswm.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/compatibility.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <imgui.h>

// fightingengine headers
#include "engine/application.hpp"
#include "engine/audio.hpp"
#include "engine/maths_core.hpp"
#include "engine/opengl/render_command.hpp"
#include "engine/ui/profiler_panel.hpp"
#include "engine/util.hpp"
#ifdef _DEBUG
#include "thirdparty/magic_enum.hpp"
#endif // _DEBUG
using namespace fightingengine;

// game headers
#include "2d_game_object.hpp"
#include "2d_physics.hpp"
#include "console.hpp"
#include "opengl/sprite_renderer.hpp"
#include "spritemap.hpp"
using namespace game2d;

float screen_width = 720.0f;
float screen_height = 480.0f;
bool show_game_info = true;
bool show_profiler = true;
bool show_console = false;
bool show_demo_window = false;
bool advance_one_frame = false;
bool juice_game = false;
// key bindings: application
SDL_Scancode key_quit = SDL_SCANCODE_ESCAPE;
SDL_Scancode key_console = SDL_SCANCODE_F12;
SDL_Scancode key_fullscreen = SDL_SCANCODE_F;
SDL_Scancode key_advance_one_frame = SDL_SCANCODE_RSHIFT;
SDL_Scancode key_advance_one_frame_held = SDL_SCANCODE_F10;
SDL_Scancode key_force_gameover = SDL_SCANCODE_F11;

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

// Physics tick
int PHYSICS_TICKS_PER_SECOND = 40;
float SECONDS_PER_PHYSICS_TICK = 1.0f / PHYSICS_TICKS_PER_SECOND;
float seconds_since_last_physics_tick = 0;

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
sprite::type player_sprite = sprite::type::TREE_4;
sprite::type bullet_sprite = sprite::type::ROCKET_2;
sprite::type wall_sprite = sprite::type::WALL_BIG;
// entity: enemy
float seconds_until_max_difficulty = 10.0f;
float seconds_until_max_difficulty_spent = 0.0f;
float wall_seconds_between_spawning_start = 0.5f;
float wall_seconds_between_spawning_current = wall_seconds_between_spawning_start;
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

static GameObject2D
create_camera()
{
  GameObject2D game_object;
  game_object.pos = glm::vec2{ 0.0f, 0.0f };
  return game_object;
}

static void
update_position(GameObject2D& camera, const KeysAndState& keys, Application& app, float delta_time_s)
{
  // go.pos = glm::vec2(other.pos.x - screen_width / 2.0f, other.pos.y - screen_height / 2.0f);
  if (app.get_input().get_key_held(keys.key_camera_left))
    camera.pos.x -= delta_time_s * camera.speed_current;
  if (app.get_input().get_key_held(keys.key_camera_right))
    camera.pos.x += delta_time_s * camera.speed_current;
  if (app.get_input().get_key_held(keys.key_camera_up))
    camera.pos.y -= delta_time_s * camera.speed_current;
  if (app.get_input().get_key_held(keys.key_camera_down))
    camera.pos.y += delta_time_s * camera.speed_current;
};

}; // namespace camera

namespace bullet {

static GameObject2D
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
  game_object.colour = bullet_colour;

  game_object.do_lifecycle_timed = true;
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

static GameObject2D
create_player()
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
  game_object.invulnerable = true;
  game_object.hits_able_to_be_taken = 10;
  game_object.colour = player_colour;

  return game_object;
};

static void
update_input(GameObject2D& obj, KeysAndState& keys, Application& app, GameObject2D& camera)
{
  keys.l_analogue_x = 0.0f;
  keys.l_analogue_y = 0.0f;
  keys.shoot_pressed = false;
  keys.boost_pressed = false;
  keys.pause_pressed = false;

  // Keymaps: keyboard
  if (keys.use_keyboard) {
    if (app.get_input().get_key_held(keys.w)) {
      keys.l_analogue_y = -1.0f;
    } else if (app.get_input().get_key_held(keys.s)) {
      keys.l_analogue_y = 1.0f;
    } else {
      keys.l_analogue_y = 0.0f;
    }

    if (app.get_input().get_key_held(keys.a)) {
      keys.l_analogue_x = -1.0f;
    } else if (app.get_input().get_key_held(keys.d)) {
      keys.l_analogue_x = 1.0f;
    } else {
      keys.l_analogue_x = 0.0f;
    }

    keys.shoot_pressed = app.get_input().get_mouse_lmb_held();
    keys.boost_pressed = app.get_input().get_key_held(keys.key_boost);
    keys.pause_pressed = app.get_input().get_key_down(keys.key_pause);

    glm::vec2 player_world_space_pos = gameobject_in_worldspace(camera, obj);
    float mouse_angle_around_player = atan2(app.get_input().get_mouse_pos().y - player_world_space_pos.y,
                                            app.get_input().get_mouse_pos().x - player_world_space_pos.x);
    mouse_angle_around_player += HALF_PI;
    obj.angle_radians = mouse_angle_around_player;
  }

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

// process input
// ability: boost
// look in mouse direction
// shoot
static void
update_game_logic(GameObject2D& obj,
                  const KeysAndState& keys,
                  std::vector<GameObject2D>& bullets,
                  float delta_time_s,
                  ALint source_id)
{
  // process input
  obj.velocity.x = keys.l_analogue_x;
  obj.velocity.y = keys.l_analogue_y;
  obj.velocity *= obj.speed_current;

  // Ability: Boost

  if (keys.boost_pressed) {
    // Boost when shift pressed
    obj.shift_boost_time_left -= delta_time_s;
    obj.shift_boost_time_left = obj.shift_boost_time_left < 0.0f ? 0.0f : obj.shift_boost_time_left;
  } else {
    // Recharge when shift released
    obj.shift_boost_time_left += delta_time_s;
    // Cap limit
    obj.shift_boost_time_left =
      obj.shift_boost_time_left > obj.shift_boost_time ? obj.shift_boost_time : obj.shift_boost_time_left;
  }

  if (keys.boost_pressed && obj.shift_boost_time_left > 0.0f) {
    obj.velocity *= obj.velocity_boost_modifier;
  }

  // look in mouse direction
  bool look_at_mouse = true;
  if (look_at_mouse) {
    // obj.angle_radians = look_angle;
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

  // Ability: Shoot
  // if (keys.shoot_pressed)
  //   obj.bullets_to_fire_after_releasing_mouse_left = obj.bullets_to_fire_after_releasing_mouse;

  if (obj.bullet_seconds_between_spawning_left > 0.0f)
    obj.bullet_seconds_between_spawning_left -= delta_time_s;

  if (obj.bullet_seconds_between_spawning_left <= 0.0f) {
    obj.bullet_seconds_between_spawning_left = obj.bullet_seconds_between_spawning;
    // obj.bullets_to_fire_after_releasing_mouse_left -= 1;
    // obj.bullets_to_fire_after_releasing_mouse_left =
    //   obj.bullets_to_fire_after_releasing_mouse_left < 0 ? 0 : obj.bullets_to_fire_after_releasing_mouse_left;

    // spawn bullet

    GameObject2D bullet_copy = bullet::create_bullet();

    glm::vec2 bullet_pos = obj.pos;
    bullet_pos.x += obj.size.x / 2.0f - bullet_copy.size.x / 2.0f;
    bullet_pos.y += obj.size.y / 2.0f - bullet_copy.size.y / 2.0f;

    // override defaults
    bullet_copy.pos = bullet_pos;
    bullet_copy.angle_radians = obj.angle_radians;
    bullet_copy.velocity.x = bullet_copy.speed_current;
    bullet_copy.velocity.y = bullet_copy.speed_current;

    bullets.push_back(bullet_copy);

    audio::play_sound(source_id);
  }
}

} // namespace player

namespace enemy {

static GameObject2D
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
  game_object.colour = wall_colour;
  return game_object;
};

// spawn a random enemy every X seconds
static void
enemy_spawner(std::vector<GameObject2D>& enemies,
              GameObject2D& camera,
              RandomState& rnd,
              float screen_height,
              float screen_width,
              float delta_time_s)
{

  wall_seconds_between_spawning_left -= delta_time_s;
  if (wall_seconds_between_spawning_left <= 0.0f) {
    wall_seconds_between_spawning_left = wall_seconds_between_spawning_current;

    // glm::ivec2 mouse_pos = app.get_input().get_mouse_pos();
    // printf("(game) mmb clicked %i %i \n", mouse_pos.x, mouse_pos.y);
    // glm::vec2 world_pos = glm::vec2(mouse_pos) + camera.pos;

    glm::vec2 rnd_pos =
      glm::vec2(rand_det_s(rnd.rng, 0.0f, 1.0f) * screen_width, rand_det_s(rnd.rng, 0.0f, 1.0f) * screen_height);
    glm::vec2 world_pos = rnd_pos + camera.pos;

    GameObject2D wall_copy = create_enemy();
    wall_copy.pos = world_pos; // override defaults
    enemies.push_back(wall_copy);
  }

  // increase difficulty
  // 0.5 is starting cooldown
  // after 30 seconds, cooldown should be 0
  const float end_cooldown = 0.2f;
  seconds_until_max_difficulty_spent += delta_time_s;
  float percent = glm::clamp(seconds_until_max_difficulty_spent / seconds_until_max_difficulty, 0.0f, 1.0f);
  wall_seconds_between_spawning_current = glm::mix(wall_seconds_between_spawning_start, end_cooldown, percent);
};

// rotate to player
// move to player
static void
enemy_chase_player(std::vector<GameObject2D>& objs, GameObject2D& player, float delta_time_s)
{
  //
  // simple chase ai
  //

  for (auto& obj : objs) {
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
}

} // namespace: enemy

int
main()
{
#ifdef WIN32
#include <Windows.h>
  ::ShowWindow(::GetConsoleWindow(), SW_HIDE); // hide console
#endif

  std::cout << "booting up..." << std::endl;
  const auto app_start = std::chrono::high_resolution_clock::now();

  RandomState rnd;
  Application app("2D Game", static_cast<int>(screen_width), static_cast<int>(screen_height));
  // SDL_GL_SetSwapInterval(1); // VSync
  Profiler profiler;
  Console console;

  // controllers
  // could improve: currently only checks for controller once at start of app
  // bool use_keyboard = true;
  // SDL_GameController* controller = NULL;
  // for (int i = 0; i < SDL_NumJoysticks(); ++i) {
  //   if (SDL_IsGameController(i)) {
  //     controller = SDL_GameControllerOpen(i);
  //     if (controller) {
  //       use_keyboard = false;
  //       break;
  //     } else {
  //       fprintf(stderr, "Could not open gamecontroller %i: %s\n", i, SDL_GetError());
  //     }
  //   }
  // }

  // textures

  std::vector<std::pair<int, std::string>> textures_to_load;
  textures_to_load.emplace_back(tex_unit_kenny_nl,
                                "assets/textures/kennynl_1bit_pack/Tilesheet/monochrome_transparent_packed.png");
  load_textures_threaded(textures_to_load, app_start);

  // sound

  float master_volume = 0.1f;
  audio::init_al(); // audio setup, which opens one device and one context

  // audio buffers e.g. sound effects
  ALuint audio_gunshot_0 = audio::load_sound("assets/audio/seb/Gun_03_shoot.wav");
  ALuint audio_enemy_hit = audio::load_sound("assets/audio/seb/Impact_03.wav");
  ALuint audio_menu_0 = audio::load_sound("assets/audio/menu-8-bit-adventure.wav");
  ALuint audio_game_0 = audio::load_sound("assets/audio/game2-downforce.wav");
  ALuint audio_game_1 = audio::load_sound("assets/audio/game1-sawtines.wav");

  // audio source e.g. sheep with position.
  ALuint audio_source_enemy_hit;
  alGenSources(1, &audio_source_enemy_hit);
  alSourcei(audio_source_enemy_hit, AL_BUFFER, (ALint)audio_enemy_hit);
  alSourcef(audio_source_enemy_hit, AL_GAIN, master_volume);

  ALuint audio_source_bullet;
  alGenSources(1, &audio_source_bullet);                             // generate source
  alSourcei(audio_source_bullet, AL_BUFFER, (ALint)audio_gunshot_0); // attach buffer to source
  alSourcef(audio_source_bullet, AL_GAIN, master_volume / 2.0f);     // set volume

  // alSourcef(audio_source_continuous_music, AL_PITCH, 1.0f); // set pitch
  // ALenum audio_state;   // get state of source
  // alGetSourcei(audio_source_continuous_music, AL_SOURCE_STATE, &audio_state);
  // ALfloat audio_offset; // get offset of source
  // alGetSourcef(audio_source_continuous_music, AL_SEC_OFFSET, &audio_offset);

  log_time_since("(INFO) Audio Loaded ", app_start);

  // Rendering

  glm::mat4 projection = glm::ortho(0.0f, screen_width, screen_height, 0.0f, -1.0f, 1.0f);

  RenderCommand::init();
  RenderCommand::set_viewport(0, 0, static_cast<uint32_t>(screen_width), static_cast<uint32_t>(screen_height));
  RenderCommand::set_depth_testing(false); // disable depth testing for 2d
  sprite_renderer::init();

  Shader colour_shader = Shader("shaders_2d/2d_basic.vert", "shaders_2d/2d_colour.frag");
  colour_shader.bind();
  colour_shader.set_vec4("colour", chosen_colour_1);

  Shader instanced_quad_shader = Shader("shaders_2d/2d_instanced.vert", "shaders_2d/2d_instanced.frag");
  instanced_quad_shader.bind();
  instanced_quad_shader.set_mat4("projection", projection);
  instanced_quad_shader.set_int("tex", tex_unit_kenny_nl);

  // game

  //   GameObject2D logo_entity;
  //   logo_entity.sprite = logo_sprite;
  //   logo_entity.tex_slot = tex_unit_kenny_nl;
  //   logo_entity.name = "logo";
  //   logo_entity.pos = { screen_width / 2.0f, screen_height / 2.0f };
  //   logo_entity.size = { 4.0f * 768.0f / 48.0f, 4.0f * 362.0f / 22.0f };

  GameObject2D tex_obj;
  tex_obj.tex_slot = tex_unit_kenny_nl;
  tex_obj.name = "texture_sheet";
  tex_obj.pos = { 0.0f, 20.0f };
  tex_obj.size = { 768.0f, 352.0f };
  tex_obj.colour = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
  tex_obj.angle_radians = 0.0;
  tex_obj.sprite = sprite::type::EMPTY;

  // entities

  GameObject2D camera = camera::create_camera();

  std::vector<GameObject2D> entities_walls;
  std::vector<GameObject2D> entities_bullets;
  std::vector<GameObject2D> entities_player;
  std::vector<KeysAndState> player_keys;

  { // populate defaults
    GameObject2D player0 = player::create_player();
    GameObject2D player1 = player::create_player();

    entities_player.push_back(player0);
    // entities_player.push_back(player1);

    KeysAndState player0_keys;
    player0_keys.use_keyboard = true;
    KeysAndState player1_keys;
    player1_keys.use_keyboard = true;
    player1_keys.w = SDL_Scancode::SDL_SCANCODE_I;
    player1_keys.s = SDL_Scancode::SDL_SCANCODE_K;
    player1_keys.a = SDL_Scancode::SDL_SCANCODE_J;
    player1_keys.d = SDL_Scancode::SDL_SCANCODE_L;
    player1_keys.key_boost = SDL_Scancode::SDL_SCANCODE_RCTRL;

    player_keys.push_back(player0_keys);
    // player_keys.push_back(player1_keys);
  }

  uint32_t objects_destroyed = 0;

  std::cout << "GameObject2D is " << sizeof(GameObject2D) << " bytes" << std::endl;
  log_time_since("(INFO) End Setup ", app_start);

  // ---- App ----

  while (app.is_running()) {
    profiler.new_frame();
    profiler.begin(Profiler::Stage::UpdateLoop);

    app.frame_begin(); // get input events
    float delta_time_s = app.get_delta_time();
    if (delta_time_s >= 0.25f)
      delta_time_s = 0.25f;

    profiler.begin(Profiler::Stage::Physics);
    {

      if (state == GameState::GAME_ACTIVE || (state == GameState::GAME_PAUSED && advance_one_frame)) {

        // FIXED PHYSICS TICK
        seconds_since_last_physics_tick += delta_time_s;
        while (seconds_since_last_physics_tick >= SECONDS_PER_PHYSICS_TICK) {
          seconds_since_last_physics_tick -= SECONDS_PER_PHYSICS_TICK;

          // hopefully no copy of GameObject2D
          std::vector<std::reference_wrapper<GameObject2D>> collidable;
          collidable.insert(collidable.end(), entities_walls.begin(), entities_walls.end());
          collidable.insert(collidable.end(), entities_bullets.begin(), entities_bullets.end());
          collidable.insert(collidable.end(), entities_player.begin(), entities_player.end());

          // broadphase: detect collisions can actually happen and discard collisions which can't.
          // sort and prune algorithm. note: suffers from large worlds with inactive objects.
          // this issue can be solved by using multiple smaller SAP's which form a grid.
          // note: i've adjusted this algortihm to do 2-axis SAP.

          // Do broad-phase check.
          std::map<uint64_t, Collision2D> collisions;

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

          //
          // game's response
          //

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
                  audio::play_sound(audio_source_enemy_hit);

                  // other object was player?
                  for (int j = 0; j < entities_player.size(); j++) {
                    GameObject2D& player = entities_player[j];
                    if (id_0 == player.id || id_1 == player.id) {
                      std::cout << "player" << j << " hit taken: " << std::endl;
                      player.hits_taken += 1;
                    }
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
    }
    profiler.end(Profiler::Stage::Physics);
    profiler.begin(Profiler::Stage::SdlInput);
    {
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

      // Settings: Exit App
      if (app.get_input().get_key_down(key_quit))
        app.shutdown();

#ifdef _DEBUG

      // Debug: Advance one frame
      if (app.get_input().get_key_down(key_advance_one_frame)) {
        advance_one_frame = true;
      }
      // Debug: Advance frames
      if (app.get_input().get_key_held(key_advance_one_frame_held)) {
        advance_one_frame = true;
      }
      // Debug: Force game over
      if (app.get_input().get_key_down(key_force_gameover)) {
        state = GameState::GAME_OVER_SCREEN;
      }

      // Settings: Toggle Console
      if (app.get_input().get_key_down(key_console))
        show_console = !show_console;

      // Shader hot reloading
      // if (app.get_input().get_key_down(SDL_SCANCODE_R)) {
      //   reload_shader_program(&fun_shader.ID, "2d_texture.vert", "effects/posterized_water.frag");
      //   fun_shader.bind();
      //   fun_shader.set_mat4("projection", projection);
      //   fun_shader.set_int("tex", tex_unit_kenny_nl);
      // }

      if (app.get_input().get_key_down(SDL_SCANCODE_BACKSPACE)) {
        entities_player.pop_back(); // kill the first player >:(
        player_keys.pop_back();
      }
#endif // _DEBUG
    }
    profiler.end(Profiler::Stage::SdlInput);
    profiler.begin(Profiler::Stage::GameTick);
    {

      // Update player's input

      for (int i = 0; i < entities_player.size(); i++) {
        GameObject2D& player = entities_player[i];
        KeysAndState& keys = player_keys[i];

        player::update_input(player, keys, app, camera);

        if (keys.pause_pressed)
          state = state == GameState::GAME_PAUSED ? GameState::GAME_ACTIVE : GameState::GAME_PAUSED;
      }

      // Update game state

      if (state == GameState::GAME_ACTIVE) {

        // update: players

        for (int i = 0; i < entities_player.size(); i++) {
          GameObject2D& player = entities_player[i];
          KeysAndState& keys = player_keys[i];

          player::update_game_logic(player, keys, entities_bullets, delta_time_s, audio_source_bullet);

          bool player_alive = player.invulnerable || player.hits_taken < player.hits_able_to_be_taken;
          if (!player_alive)
            state = GameState::GAME_OVER_SCREEN;

          gameobject::update_position(player, delta_time_s);
        }

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

        // update: spawn enemies

        size_t players_in_game = entities_player.size();
        if (players_in_game > 0) {

          // for the moment, eat player 0
          GameObject2D player_to_chase = entities_player[0];
          enemy::enemy_chase_player(entities_walls, player_to_chase, delta_time_s);

          //... and only spawn enemies if there is a player.
          enemy::enemy_spawner(entities_walls, camera, rnd, screen_height, screen_width, delta_time_s);
        }
      }

      // todo: manage lifecycle and delete expired objects
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
        renderables.insert(renderables.end(), entities_player.begin(), entities_player.end());

        for (std::reference_wrapper<GameObject2D> obj : renderables) {
          sprite_renderer::draw_sprite_debug(
            camera, screen_wh, instanced_quad_shader, obj.get(), colour_shader, debug_line_colour);
        }

        // draw the spritesheet for reference
        sprite_renderer::draw_sprite_debug(
          camera, screen_wh, instanced_quad_shader, tex_obj, colour_shader, debug_line_colour);

        // sprite_renderer::draw_instanced_sprite(camera,
        //                                        screen_wh,
        //                                        instanced_quad_shader,
        //                                        player1,
        //                                        glm::vec4(1.0f, 0.0f, 0.0f, 1.0f),
        //                                        glm::vec4(1.0f, 0.0f, 0.0f, 1.0f),
        //                                        glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
        //                                        glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
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
          ImGui::Text("camera pos %f %f", camera.pos.x, camera.pos.y);
          ImGui::Text("mouse pos %f %f", app.get_input().get_mouse_pos().x, app.get_input().get_mouse_pos().y);
          ImGui::Separator();

          for (int i = 0; i < entities_player.size(); i++) {
            GameObject2D& player = entities_player[i];
            ImGui::Text("player id: %i", player.id);
            ImGui::Text("pos %f %f", player.pos.x, player.pos.y);
            ImGui::Text("vel x: %f y: %f", player.velocity.x, player.velocity.y);
            ImGui::Text("angle %f", player.angle_radians);
            ImGui::Text("hp_max %i", player.hits_able_to_be_taken);
            ImGui::Text("hits taken %i", player.hits_taken);
            ImGui::Text("boost %f", player.shift_boost_time_left);
            ImGui::Separator();
          }

          // ImGui::Text("highscore: %i", highscore);
          ImGui::Text("Walls: %i", entities_walls.size());
          ImGui::Text("Bullets: %i", entities_bullets.size());
          ImGui::Text("(game) destroyed: %i", objects_destroyed);
          ImGui::Text("(game) enemy spawn rate: %f", wall_seconds_between_spawning_current);
#ifdef _DEBUG
          auto state_name = magic_enum::enum_name(state);
          ImGui::Text("(game) state: %s", std::string(state_name).data());
#endif // _DEBUG

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
      app.frame_end();
    }
    profiler.end(Profiler::Stage::FrameEnd);
    profiler.end(Profiler::Stage::UpdateLoop);
  }

  // end app running
}
