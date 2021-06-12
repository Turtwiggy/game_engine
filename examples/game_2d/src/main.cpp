//
// A 2D thing.
//

// c++ lib headers
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
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
#include "engine/grid.hpp"
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

enum class GameState
{
  GAME_SPLASH_SCREEN,
  GAME_OVER_SCREEN,
  GAME_ACTIVE,
  GAME_PAUSED
};
GameState state = GameState::GAME_ACTIVE;

glm::ivec2 screen_wh = { 1280, 720 };
bool show_game_info = true;
bool show_profiler = true;
bool show_windows_console = false;
bool show_game_console = false;
bool show_imgui_demo_window = false;
bool debug_advance_one_frame = false;
bool app_fullscreen = false;
bool app_mute_sfx = true;
bool app_use_vsync = true;
bool app_limit_framerate = false;
// game config
bool show_entity_menu = true;
bool game_spawn_enemies = false;
bool game_ui_show_inventory = true;
bool game_player_shoot = true; // affects all players
bool game_destroy_half_sprites_on_damage = false;
bool render_spritesheet = false;
float seconds_until_max_difficulty = 100.0f;
float seconds_until_max_difficulty_spent = 0.0f;
float wall_seconds_between_spawning_start = 0.5f;
float wall_seconds_between_spawning_current = wall_seconds_between_spawning_start;
float wall_seconds_between_spawning_left = 0.0f;
float wall_seconds_between_spawning_end = 0.2f;
const float safe_radius_around_player = 7500.0f;
const float enemy_default_speed = 60.0f;
const float player_default_speed = 50.0f;
const float bullet_seconds_between_spawning_default = 1.0f;
const int enemy_hits_to_take = 1;
const float enemy_direct_attack_threshold = 4000.0f;
// key bindings: application
SDL_Scancode key_quit = SDL_SCANCODE_ESCAPE;
SDL_Scancode key_console = SDL_SCANCODE_F12;
SDL_Scancode key_fullscreen = SDL_SCANCODE_F;
SDL_Scancode key_advance_one_frame = SDL_SCANCODE_RSHIFT;
SDL_Scancode key_advance_one_frame_held = SDL_SCANCODE_F10;
SDL_Scancode key_force_gameover = SDL_SCANCODE_F11;
// physics tick
int PHYSICS_GRID_SIZE = 100;
int PHYSICS_TICKS_PER_SECOND = 40;
float SECONDS_PER_PHYSICS_TICK = 1.0f / PHYSICS_TICKS_PER_SECOND;
float seconds_since_last_physics_tick = 0;

// textures
const int tex_unit_kenny_nl = 0;

// default colour palette; https://colorhunt.co/palette/273312
// normal palette
const glm::vec4 PALETTE_COLOUR_1_1 = glm::vec4(57.0f / 255.0f, 62.0f / 255.0f, 70.0f / 255.0f, 1.0f);    // black
const glm::vec4 PALETTE_COLOUR_2_1 = glm::vec4(0.0f / 255.0f, 173.0f / 255.0f, 181.0f / 255.0f, 1.0f);   // blue
const glm::vec4 PALETTE_COLOUR_3_1 = glm::vec4(170.0f / 255.0f, 216.0f / 255.0f, 211.0f / 255.0f, 1.0f); // lightblue
const glm::vec4 PALETTE_COLOUR_4_1 = glm::vec4(238.0f / 255.0f, 238.0f / 255.0f, 238.0f / 255.0f, 1.0f); // grey
// damage palette
const glm::vec4 PALETTE_COLOUR_1_0 = glm::vec4(255.0f / 255.0f, 201.0f / 255.0f, 150.0f / 255.0f, 1.0f); // yellowish
const glm::vec4 PALETTE_COLOUR_2_0 = glm::vec4(255.0f / 255.0f, 132.0f / 255.0f, 116.0f / 255.0f, 1.0f); // orangish
const glm::vec4 PALETTE_COLOUR_3_0 = glm::vec4(159.0f / 255.0f, 95.0f / 255.0f, 128.0f / 255.0f, 1.0f);  // lightpurple
const glm::vec4 PALETTE_COLOUR_4_0 = glm::vec4(88.0f / 255.0f, 61.0f / 255.0f, 114.0f / 255.0f, 1.0f);   // darkpurple
// industry palette
const glm::vec4 PALETTE_COLOUR_1_2 = glm::vec4(253.0f / 255.0f, 225.0f / 255.0f, 168.0f / 255.0f, 1.0f); // lightyellow
const glm::vec4 PALETTE_COLOUR_2_2 = glm::vec4(255.0f / 255.0f, 210.0f / 255.0f, 103.0f / 255.0f, 1.0f); // darkyellow
const glm::vec4 PALETTE_COLOUR_3_2 = glm::vec4(255.0f / 255.0f, 113.0f / 255.0f, 95.0f / 255.0f, 1.0f);  // lightorange
const glm::vec4 PALETTE_COLOUR_4_2 = glm::vec4(118.0f / 255.0f, 56.0f / 255.0f, 75.0f / 255.0f, 1.0f);   // darkorange
const glm::vec4 PALETTE_COLOUR_5_2 = glm::vec4(39.0f / 255.0f, 72.0f / 255.0f, 88.0f / 255.0f, 1.0f); // lightdarkblue
const glm::vec4 PALETTE_COLOUR_6_2 = glm::vec4(22.0f / 255.0f, 56.0f / 255.0f, 73.0f / 255.0f, 1.0f); // darkdarkblue
const glm::vec4 PALETTE_COLOUR_7_2 = glm::vec4(20.0f / 255.0f, 36.0f / 255.0f, 45.0f / 255.0f, 1.0f); // blackish
// chosen colours
glm::vec4 chosen_colour_0 = PALETTE_COLOUR_1_1;
glm::vec4 chosen_colour_1 = PALETTE_COLOUR_2_1;
glm::vec4 chosen_colour_2 = PALETTE_COLOUR_3_1;
glm::vec4 chosen_colour_3 = PALETTE_COLOUR_4_1;
// entity colours
glm::vec4 background_colour = chosen_colour_0;     // black
glm::vec4 debug_line_colour = chosen_colour_1;     // blue
glm::vec4 player_colour = chosen_colour_1;         // blue
glm::vec4 player_dead_colour = PALETTE_COLOUR_2_0; //
glm::vec4 bullet_colour = chosen_colour_2;         // lightblue
glm::vec4 wall_colour = chosen_colour_3;           // grey
glm::vec4 logo_entity_colour = chosen_colour_3;    // grey
// entity sprites
sprite::type sprite_logo = sprite::type::WALL_BIG;
sprite::type sprite_player = sprite::type::PERSON_1;
sprite::type sprite_bullet = sprite::type::WEAPON_ARROW_1;
sprite::type sprite_enemy_core = sprite::type::PERSON_2;
sprite::type sprite_enemy_muncher = sprite::type::ORC;
sprite::type sprite_fire = sprite::type::FIRE;

namespace camera {

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

static void
update_input(GameObject2D& obj, KeysAndState& keys, Application& app, GameObject2D& camera)
{
  keys.l_analogue_x = 0.0f;
  keys.l_analogue_y = 0.0f;
  keys.r_analogue_x = 0.0f;
  keys.r_analogue_y = 0.0f;
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

    float x_axis = glm::sin(mouse_angle_around_player);
    float y_axis = -glm::cos(mouse_angle_around_player);
    keys.r_analogue_x = x_axis;
    keys.r_analogue_y = y_axis;
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
// update colour
static void
update_game_logic(GameObject2D& player,
                  const KeysAndState& keys,
                  std::vector<GameObject2D>& bullets,
                  float delta_time_s,
                  ALint source_id)
{
  // process input
  player.velocity.x = keys.l_analogue_x;
  player.velocity.y = keys.l_analogue_y;
  player.velocity *= player.speed_current;

  // Ability: Boost

  if (keys.boost_pressed) {
    // Boost when shift pressed
    player.shift_boost_time_left -= delta_time_s;
    player.shift_boost_time_left = player.shift_boost_time_left < 0.0f ? 0.0f : player.shift_boost_time_left;
  } else {
    // Recharge when shift released
    player.shift_boost_time_left += delta_time_s;
    // Cap limit
    player.shift_boost_time_left =
      player.shift_boost_time_left > player.shift_boost_time ? player.shift_boost_time : player.shift_boost_time_left;
  }

  if (keys.boost_pressed && player.shift_boost_time_left > 0.0f) {
    player.velocity *= player.velocity_boost_modifier;
  }

  // // look in mouse direction
  // bool look_at_mouse = true;
  // if (look_at_mouse) {
  //   // obj.angle_radians = look_angle;
  // }
  // // look in velocity direction
  // else {
  //   if (glm::length2(obj.velocity) > 0) {
  //     glm::vec2 up_axis = glm::vec2(0.0, -1.0);
  //     float unsigned_angle = glm::angle(up_axis, obj.velocity);
  //     float sign = (up_axis.x * obj.velocity.y - up_axis.y * obj.velocity.x) >= 0.0f ? 1.0f : -1.0f;
  //     float signed_angle = unsigned_angle * sign;

  //     obj.angle_radians = signed_angle;
  //   }
  // }

  // Ability: Shoot
  // if (keys.shoot_pressed)
  //   obj.bullets_to_fire_after_releasing_mouse_left = obj.bullets_to_fire_after_releasing_mouse;

  if (game_player_shoot) {
    if (player.bullet_seconds_between_spawning_left > 0.0f)
      player.bullet_seconds_between_spawning_left -= delta_time_s;

    if (player.bullet_seconds_between_spawning_left <= 0.0f) {
      player.bullet_seconds_between_spawning_left = player.bullet_seconds_between_spawning;
      // obj.bullets_to_fire_after_releasing_mouse_left -= 1;
      // obj.bullets_to_fire_after_releasing_mouse_left =
      //   obj.bullets_to_fire_after_releasing_mouse_left < 0 ? 0 : obj.bullets_to_fire_after_releasing_mouse_left;

      // spawn bullet

      GameObject2D bullet_copy = gameobject::create_bullet(sprite_bullet, tex_unit_kenny_nl, bullet_colour);
      // override defaults
      // fix offset issue so bullet spawns in middle of player
      glm::vec2 bullet_pos = player.pos;
      bullet_pos.x += player.size.x / 2.0f - bullet_copy.size.x / 2.0f;
      bullet_pos.y += player.size.y / 2.0f - bullet_copy.size.y / 2.0f;
      bullet_copy.pos = bullet_pos;
      // convert right analogue input to velocity
      bullet_copy.velocity.x = keys.r_analogue_x * bullet_copy.speed_current;
      bullet_copy.velocity.y = keys.r_analogue_y * bullet_copy.speed_current;

      bullets.push_back(bullet_copy);

      if (!app_mute_sfx)
        audio::play_sound(source_id);
    }
  }

  // update colour
  float t = (player.hits_taken) / static_cast<float>(player.hits_able_to_be_taken);
  t = glm::clamp(t, 0.0f, 1.0f); // clamp it
  glm::vec4 col = glm::mix(player_colour, player_dead_colour, t);
  float min_alpha = 0.7f;
  col.a = glm::clamp(1.0f - t, min_alpha, 1.0f);
  player.colour = col;
}

} // namespace player

namespace enemy {

static void
spawn_enemy(std::vector<GameObject2D>& enemies, GameObject2D& camera, glm::vec2 pos, RandomState& rnd)
{
  GameObject2D wall_copy =
    gameobject::create_enemy(sprite_enemy_core, tex_unit_kenny_nl, wall_colour, rnd, enemy_default_speed);
  // override defaults
  wall_copy.pos = pos;
  wall_copy.hits_able_to_be_taken = enemy_hits_to_take;
  enemies.push_back(wall_copy);
}

// spawn a random enemy every X seconds
static void
enemy_spawner(std::vector<GameObject2D>& enemies,
              GameObject2D& camera,
              std::vector<GameObject2D>& players,
              RandomState& rnd,
              glm::ivec2 screen_wh,
              float delta_time_s)
{

  wall_seconds_between_spawning_left -= delta_time_s;
  if (wall_seconds_between_spawning_left <= 0.0f) {
    wall_seconds_between_spawning_left = wall_seconds_between_spawning_current;

    // glm::ivec2 mouse_pos = app.get_input().get_mouse_pos();
    // printf("(game) mmb clicked %i %i \n", mouse_pos.x, mouse_pos.y);
    // glm::vec2 world_pos = glm::vec2(mouse_pos) + camera.pos;

    // search params
    bool continue_search = true;
    int iterations_max = 3;
    int iteration = 0;
    // result
    float distance_squared = 0;
    glm::vec2 found_pos = { 0.0f, 0.0f };

    // generate random pos not too close to players
    do {

      // tried to generate X times
      if (iteration == iterations_max) {
        // ah, screw it, just spawn at 0, 0
        continue_search = false;
        std::cout << "(EnemySpawner) max iterations hit" << std::endl;
      }

      bool ok = true;
      glm::vec2 rnd_pos =
        glm::vec2(rand_det_s(rnd.rng, 0.0f, 1.0f) * screen_wh.x, rand_det_s(rnd.rng, 0.0f, 1.0f) * screen_wh.y);

      for (auto& player : players) {

        distance_squared = glm::distance2(rnd_pos, player.pos);
        ok = distance_squared > safe_radius_around_player;

        if (ok) {
          continue_search = false;
          found_pos = rnd_pos;
        }
        iteration += 1;
      }

    } while (continue_search);

    // std::cout << "enemy spawning " << distance_squared << " away from player" << std::endl;
    glm::vec2 world_pos = found_pos + camera.pos;

    if (game_spawn_enemies) {
      // spawn enemy
      GameObject2D wall_copy =
        gameobject::create_enemy(sprite_enemy_core, tex_unit_kenny_nl, wall_colour, rnd, enemy_default_speed);
      // override defaults
      wall_copy.pos = world_pos;
      wall_copy.hits_able_to_be_taken = enemy_hits_to_take;
      enemies.push_back(wall_copy);
    }
  }

  // increase difficulty
  // 0.5 is starting cooldown
  // after 30 seconds, cooldown should be 0
  seconds_until_max_difficulty_spent += delta_time_s;
  float percent = glm::clamp(seconds_until_max_difficulty_spent / seconds_until_max_difficulty, 0.0f, 1.0f);
  wall_seconds_between_spawning_current =
    glm::mix(wall_seconds_between_spawning_start, wall_seconds_between_spawning_end, percent);
};

namespace ai {

static void
enemy_to_dir_vector(GameObject2D& obj, glm::vec2 dir, float delta_time_s)
{
  dir = glm::normalize(dir);
  obj.pos += (dir * obj.speed_current * delta_time_s);
}

static void
enemy_directly_to_player(GameObject2D& obj, GameObject2D& player, float delta_time_s)
{
  glm::vec2 ab = player.pos - obj.pos;
  glm::vec2 dir = glm::normalize(ab);
  enemy_to_dir_vector(obj, dir, delta_time_s);
}

// note: have a flanking manager assign angles out to these enemies
// this will mean that enemies will approach the player from all sorts of different angles.
// if far... arc angles
// if close... go direct!
// some direct anyway.
static void
enemy_arc_angles_to_player(GameObject2D& obj, GameObject2D& player, float delta_time_s)
{
  // calculate a vector ab
  glm::vec2 ab = player.pos - obj.pos;
  // calculate the point halfway between ab
  glm::vec2 half_point = obj.pos + (ab / 2.0f);
  // calculate the vector at a right angle
  glm::vec2 normal = glm::vec2(-ab.y, ab.x);

#ifdef _DEBUG
  {
    float dot = glm::abs(glm::dot(ab, normal));
    assert(dot <= 0.001f); // good enough
  }
#endif

  // expensive(?) distance calc
  float distance = glm::distance(obj.pos, player.pos);
  float half_distance = distance / 2.0f;

  // offset the midpoint via normal
  float amplitude = half_distance * sin(obj.approach_theta_degrees);
  half_point += (glm::normalize(normal) * amplitude);

  // Now create a bezier curve! use the halfpoint as the control point
  // float t = 0.5f;
  // glm::vec2 p = quadratic_curve(obj.pos, half_point, player.pos, t);

  glm::vec2 dir = glm::normalize(half_point - obj.pos);
  // std::cout << "dir x:" << dir.x << " y:" << dir.y << std::endl;

  enemy_to_dir_vector(obj, dir, delta_time_s);
}

} // namespace: ai

} // namespace: enemy

void
toggle_fullscreen(Application& app, Shader& shader)
{
  app.get_window().toggle_fullscreen(); // SDL2 window toggle
  screen_wh = app.get_window().get_size();
  RenderCommand::set_viewport(0, 0, screen_wh.x, screen_wh.y);
  glm::mat4 projection =
    glm::ortho(0.0f, static_cast<float>(screen_wh.x), static_cast<float>(screen_wh.y), 0.0f, -1.0f, 1.0f);

  shader.bind();
  shader.set_mat4("projection", projection);
}

void
hide_console()
{
#ifdef WIN32
#include <Windows.h>
  if (show_windows_console)
    ::ShowWindow(::GetConsoleWindow(), SW_HIDE); // hide console
#endif
}

int
main()
{
  hide_console();

  std::cout << "booting up..." << std::endl;
  const auto app_start = std::chrono::high_resolution_clock::now();

  RandomState rnd;
  Application app("2D Game", screen_wh.x, screen_wh.y, app_use_vsync);
  app.limit_fps = app_limit_framerate;
  app.fps_if_limited = 120.0f;
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
                                "assets/2d_game/textures/kennynl_1bit_pack/monochrome_transparent_packed.png");
  load_textures_threaded(textures_to_load, app_start);

  // sound

  float master_volume = 0.1f;
  audio::init_al(); // audio setup, which opens one device and one context

  // audio buffers e.g. sound effects
  ALuint audio_gunshot_0 = audio::load_sound("assets/2d_game/audio/seb/Gun_03_shoot.wav");
  ALuint audio_impact_0 = audio::load_sound("assets/2d_game/audio/seb/Impact_01.wav");
  ALuint audio_impact_1 = audio::load_sound("assets/2d_game/audio/seb/Impact_02.wav");
  ALuint audio_impact_2 = audio::load_sound("assets/2d_game/audio/seb/Impact_03.wav");
  // ALuint audio_menu_0 = audio::load_sound("assets/2d_game/audio/menu-8-bit-adventure.wav");
  // ALuint audio_game_0 = audio::load_sound("assets/2d_game/audio/game2-downforce.wav");
  // ALuint audio_game_1 = audio::load_sound("assets/2d_game/audio/game1-sawtines.wav");

  // audio source e.g. sheep with position.
  ALuint audio_source_bullet;
  alGenSources(1, &audio_source_bullet);                             // generate source
  alSourcei(audio_source_bullet, AL_BUFFER, (ALint)audio_gunshot_0); // attach buffer to source
  alSourcef(audio_source_bullet, AL_GAIN, master_volume / 2.0f);     // set volume
  ALuint audio_source_impact_0;
  alGenSources(1, &audio_source_impact_0);
  alSourcei(audio_source_impact_0, AL_BUFFER, (ALint)audio_impact_0);
  alSourcef(audio_source_impact_0, AL_GAIN, master_volume);
  ALuint audio_source_impact_1;
  alGenSources(1, &audio_source_impact_1);
  alSourcei(audio_source_impact_1, AL_BUFFER, (ALint)audio_impact_1);
  alSourcef(audio_source_impact_1, AL_GAIN, master_volume);
  ALuint audio_source_impact_2;
  alGenSources(1, &audio_source_impact_2);
  alSourcei(audio_source_impact_2, AL_BUFFER, (ALint)audio_impact_2);
  alSourcef(audio_source_impact_2, AL_GAIN, master_volume);
  std::vector<ALuint> audio_list_impacts = { audio_impact_0, audio_impact_1, audio_impact_2 };

  // alSourcef(audio_source_continuous_music, AL_PITCH, 1.0f); // set pitch
  // ALenum audio_state;   // get state of source
  // alGetSourcei(audio_source_continuous_music, AL_SOURCE_STATE, &audio_state);
  // ALfloat audio_offset; // get offset of source
  // alGetSourcef(audio_source_continuous_music, AL_SEC_OFFSET, &audio_offset);

  log_time_since("(INFO) Audio Loaded ", app_start);

  // Rendering

  Shader colour_shader = Shader("2d_game/shaders/2d_basic.vert", "2d_game/shaders/2d_colour.frag");
  Shader instanced_quad_shader = Shader("2d_game/shaders/2d_instanced.vert", "2d_game/shaders/2d_instanced.frag");

  { // set shader attribs
    glm::mat4 projection =
      glm::ortho(0.0f, static_cast<float>(screen_wh.x), static_cast<float>(screen_wh.y), 0.0f, -1.0f, 1.0f);

    colour_shader.bind();
    colour_shader.set_vec4("colour", chosen_colour_1);

    instanced_quad_shader.bind();
    instanced_quad_shader.set_mat4("projection", projection);
    instanced_quad_shader.set_int("tex", tex_unit_kenny_nl);
  }

  RenderCommand::init();
  RenderCommand::set_viewport(0, 0, static_cast<uint32_t>(screen_wh.x), static_cast<uint32_t>(screen_wh.y));
  RenderCommand::set_depth_testing(false); // disable depth testing for 2d
  sprite_renderer::init();

  // game

  //   GameObject2D logo_entity;
  //   logo_entity.sprite = sprite_logo;
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

  uint32_t objects_destroyed = 0;

  GameObject2D camera = gameobject::create_camera();

  std::vector<GameObject2D> entities_grid;
  std::vector<GameObject2D> entities_enemies;
  std::vector<GameObject2D> entities_bullets;
  std::vector<GameObject2D> entities_player;
  std::vector<KeysAndState> player_keys;

  { // populate defaults
    GameObject2D player0 =
      gameobject::create_player(sprite_player, tex_unit_kenny_nl, player_colour, screen_wh, player_default_speed);
    player0.bullet_seconds_between_spawning = bullet_seconds_between_spawning_default;

    GameObject2D player1 =
      gameobject::create_player(sprite_player, tex_unit_kenny_nl, player_colour, screen_wh, player_default_speed);
    player1.bullet_seconds_between_spawning = bullet_seconds_between_spawning_default;

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

    for (int i = 0; i < 10; i++) {
      for (int j = 0; j < 10; j++) {
        // populate a grid
      }
    }
    // player_keys.push_back(player1_keys);
  }

  // TEMP objects

  glm::ivec2 mouse_grid;

  GameObject2D experimental_fire =
    gameobject::create_enemy(sprite_fire, tex_unit_kenny_nl, wall_colour, rnd, enemy_default_speed);
  GameObject2D experimental_new_enemy =
    gameobject::create_enemy(sprite_enemy_muncher, tex_unit_kenny_nl, wall_colour, rnd, enemy_default_speed);

  GameObject2D placeholder_collision_object_0;
  GameObject2D placeholder_collision_object_1;

  std::cout << "GameObject2D is " << sizeof(GameObject2D) << " bytes" << std::endl;
  log_time_since("(INFO) End Setup ", app_start);

  // Run App

  while (app.is_running()) {

    Uint64 frame_start_time = SDL_GetPerformanceCounter();
    profiler.new_frame();
    profiler.begin(Profiler::Stage::UpdateLoop);

    app.frame_begin(); // get input events
    float delta_time_s = app.get_delta_time();
    if (delta_time_s >= 0.25f)
      delta_time_s = 0.25f;

    profiler.begin(Profiler::Stage::Physics);
    {
      if (state == GameState::GAME_ACTIVE || (state == GameState::GAME_PAUSED && debug_advance_one_frame)) {

        // pre-physics
        // For all entities, before their position is updated, calculate the
        // grid position that they are in.
        std::vector<std::reference_wrapper<GameObject2D>> grid_entities;
        grid_entities.insert(grid_entities.end(), entities_enemies.begin(), entities_enemies.end());
        grid_entities.insert(grid_entities.end(), entities_bullets.begin(), entities_bullets.end());
        grid_entities.insert(grid_entities.end(), entities_player.begin(), entities_player.end());

        // entities: update entities grid pos
        for (auto& e : grid_entities) {
          e.get().grid_position = grid::convert_world_space_to_grid_space(e.get().pos, PHYSICS_GRID_SIZE);
          e.get().size = { PHYSICS_GRID_SIZE, PHYSICS_GRID_SIZE };
        }
        // update mouse grid pos
        {
          mouse_grid =
            grid::convert_world_space_to_grid_space(app.get_input().get_mouse_pos() + camera.pos, PHYSICS_GRID_SIZE);
        }

        // FIXED PHYSICS TICK
        seconds_since_last_physics_tick += delta_time_s;
        while (seconds_since_last_physics_tick >= SECONDS_PER_PHYSICS_TICK) {
          seconds_since_last_physics_tick -= SECONDS_PER_PHYSICS_TICK;

          // set entities that we want collision info from
          std::vector<std::reference_wrapper<GameObject2D>> collidable;
          collidable.insert(collidable.end(), entities_enemies.begin(), entities_enemies.end());
          collidable.insert(collidable.end(), entities_bullets.begin(), entities_bullets.end());
          collidable.insert(collidable.end(), entities_player.begin(), entities_player.end());

          // generate filtered broadphase collisions.
          std::map<uint64_t, Collision2D> filtered_collisions;
          generate_filtered_broadphase_collisions(collidable, filtered_collisions);

          // TODO: narrow-phase; per-model collision? convex shapes?

          //
          // game's response
          //

          for (auto& c : filtered_collisions) {

            uint32_t id_0 = c.second.ent_id_0;
            uint32_t id_1 = c.second.ent_id_1;
            // CollisionLayer layer_0 = c.second.ent_0_layer;
            // CollisionLayer layer_1 = c.second.ent_1_layer;

            // bool found_0 = false;
            // { // search for object 1
            //   auto it = std::find_if(
            //     collidable.begin(), collidable.end(), [&id_0](const GameObject2D& obj) { return obj.id == id_0; });
            //   if (it != collidable.end()) {
            //     placeholder_collision_object_0 = *it;
            //     found_0 = true;
            //   }
            // }

            // Resolve game collision matrix...!

            // Check for entities_enemies collisions

            bool player_taken_damage = false;

            for (int i = 0; i < entities_enemies.size(); i++) {
              GameObject2D& go = entities_enemies[i];
              if (id_0 == go.id || id_1 == go.id) {

                // What to do if wall collided?

                // set it as having taken damage
                go.hits_taken += 1;
                if (go.hits_taken >= go.hits_able_to_be_taken) {
                  entities_enemies.erase(entities_enemies.begin() + i);
                  objects_destroyed += 1;

                  if (!app_mute_sfx) {
                    // choose a random impact sound
                    float r = rand_det_s(rnd.rng, 0.0f, 3.0f);
                    ALuint rnd_impact = audio_list_impacts[static_cast<int>(r)];
                    audio::play_sound(rnd_impact);
                  }

                  // other object was player?
                  for (int j = 0; j < entities_player.size(); j++) {
                    GameObject2D& player = entities_player[j];
                    if (id_0 == player.id || id_1 == player.id) {
                      std::cout << "player" << j << " hit taken: " << std::endl;
                      player.hits_taken += 1;
                      player_taken_damage = true;
                    }
                  }
                }
              }
            }

            if (player_taken_damage && game_destroy_half_sprites_on_damage) {
              // player took damage! chill out for a bit.
              // destroy half the enemies..!
              for (int i = 0; i < entities_enemies.size() / 2; i++) {
                entities_enemies.erase(entities_enemies.begin());
              }
            }

            // Check for entities_bullets collisions

            for (int i = 0; i < entities_bullets.size(); i++) {
              GameObject2D& go = entities_bullets[i];
              if (id_0 == go.id || id_1 == go.id) {

                // what to do if a bullet collided?

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
        toggle_fullscreen(app, instanced_quad_shader);
        app_fullscreen = app.get_window().get_fullscreen();
      }

      if (app.window_was_resized) {
        app.window_was_resized = false;

        screen_wh = app.get_window().get_size();
        RenderCommand::set_viewport(0, 0, screen_wh.x, screen_wh.y);
        glm::mat4 projection =
          glm::ortho(0.0f, static_cast<float>(screen_wh.x), static_cast<float>(screen_wh.y), 0.0f, -1.0f, 1.0f);

        instanced_quad_shader.bind();
        instanced_quad_shader.set_mat4("projection", projection);
      }

      // Settings: Exit App
      if (app.get_input().get_key_down(key_quit))
        app.shutdown();

#ifdef _DEBUG

      // Debug: Advance one frame
      if (app.get_input().get_key_down(key_advance_one_frame)) {
        debug_advance_one_frame = true;
      }
      // Debug: Advance frames
      if (app.get_input().get_key_held(key_advance_one_frame_held)) {
        debug_advance_one_frame = true;
      }
      // Debug: Force game over
      if (app.get_input().get_key_down(key_force_gameover)) {
        state = GameState::GAME_OVER_SCREEN;
      }

      // Settings: Toggle Console
      if (app.get_input().get_key_down(key_console))
        show_game_console = !show_game_console;

      // Shader hot reloading
      // if (app.get_input().get_key_down(SDL_SCANCODE_R)) {
      //   reload_shader_program(&fun_shader.ID, "2d_texture.vert", "effects/posterized_water.frag");
      //   fun_shader.bind();
      //   fun_shader.set_mat4("projection", projection);
      //   fun_shader.set_int("tex", tex_unit_kenny_nl);
      // }

      float mousewheel = app.get_input().get_mousewheel_y();
      float epsilon = 0.0001f;
      if (mousewheel > epsilon || mousewheel < -epsilon) {
        PHYSICS_GRID_SIZE += mousewheel; // works well enough for now
        PHYSICS_GRID_SIZE = glm::max(PHYSICS_GRID_SIZE, 0);
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
          gameobject::update_position(obj, delta_time_s);

          // to improve: look in velocity direction
          {
            float angle = atan2(obj.velocity.y, obj.velocity.x);
            angle += HALF_PI + sprite::spritemap::get_sprite_rotation_offset(obj.sprite);
            obj.angle_radians = angle;
          }

          // to improve: lifecycle
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

          // update with ai behaviour
          for (auto& obj : entities_enemies) {

            // check every frame: close to player?
            float distance_squared = glm::distance2(obj.pos, player_to_chase.pos);
            if (distance_squared < enemy_direct_attack_threshold) {
              obj.ai_current = ai_behaviour::MOVEMENT_DIRECT;
            } else {
              obj.ai_current = obj.ai_original;
            }

            // check every frame: update ai behaviour
            if (obj.ai_current == ai_behaviour::MOVEMENT_DIRECT) {
              enemy::ai::enemy_directly_to_player(obj, player_to_chase, delta_time_s);
            } else if (obj.ai_current == ai_behaviour::MOVEMENT_ARC_ANGLE) {
              enemy::ai::enemy_arc_angles_to_player(obj, player_to_chase, delta_time_s);
            }
          }

          //... and only spawn enemies if there is a player.
          enemy::enemy_spawner(entities_enemies, camera, entities_player, rnd, screen_wh, delta_time_s);

          // update camera pos
          camera::update_position(camera, player_keys[0], app, delta_time_s);
        }

        // update: fire
        experimental_fire.pos = { screen_wh.x / 2.0f - 100.0f, screen_wh.y / 2.0f };

        // update: new experimental enemy
        experimental_new_enemy.pos = { screen_wh.x / 2.0f + 200.0f, screen_wh.y / 2.0f };

        // todo: manage lifecycle and delete expired objects
      }
    }
    profiler.end(Profiler::Stage::GameTick);
    profiler.begin(Profiler::Stage::Render);
    {

      RenderCommand::set_clear_colour(background_colour);
      RenderCommand::clear();
      sprite_renderer::reset_stats();
      sprite_renderer::begin_batch();
      instanced_quad_shader.bind();

      if (state == GameState::GAME_ACTIVE || state == GameState::GAME_PAUSED) {

        std::vector<std::reference_wrapper<GameObject2D>> renderables;
        renderables.insert(renderables.end(), entities_enemies.begin(), entities_enemies.end());
        renderables.insert(renderables.end(), entities_bullets.begin(), entities_bullets.end());
        renderables.insert(renderables.end(), entities_player.begin(), entities_player.end());
        renderables.push_back(experimental_fire);
        renderables.push_back(experimental_new_enemy);

        for (std::reference_wrapper<GameObject2D> obj : renderables) {
          sprite_renderer::draw_sprite_debug(
            camera, screen_wh, instanced_quad_shader, obj.get(), colour_shader, debug_line_colour);
        }

#ifdef _DEBUG
        if (render_spritesheet) {
          // draw the spritesheet for reference
          sprite_renderer::draw_sprite_debug(
            camera, screen_wh, instanced_quad_shader, tex_obj, colour_shader, debug_line_colour);
        }
#endif

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
        ImGui::Text("%.2f FPS (%.2f ms)", ImGui::GetIO().Framerate, 1000.0f / ImGui::GetIO().Framerate);
        // ImGui::SameLine(ImGui::GetWindowWidth());

        bool temp = false;

        { // limit framerate
          temp = app_limit_framerate;
          ImGui::Checkbox("Limit Framerate", &temp);
          if (temp != app_limit_framerate) {
            std::cout << "Limit fps toggled to: " << temp << std::endl;
            app.limit_fps = temp;
          }
          app_limit_framerate = temp;
        }

        { // mute sfx
          temp = app_mute_sfx;
          ImGui::Checkbox("Mute SFX", &temp);
          if (temp != app_mute_sfx) {
            std::cout << "sfx toggled to: " << temp << std::endl;
          }
          app_mute_sfx = temp;
        }

        { // use vsync
          temp = app_use_vsync;
          ImGui::Checkbox("VSync", &temp);
          if (temp != app_use_vsync) {
            std::cout << "vsync toggled to: " << temp << std::endl;
            app.get_window().set_vsync_opengl(temp);
          }
          app_use_vsync = temp;
        }

        { // toggle fullsceren
          temp = app_fullscreen;
          ImGui::Checkbox("Fullscreen", &app_fullscreen);
          if (temp != app_fullscreen) {
            std::cout << "app_fullscreen toggled to: " << temp << std::endl;
            toggle_fullscreen(app, instanced_quad_shader);
          }
          app_fullscreen = temp;
        }

        if (ImGui::MenuItem("Quit", "Esc"))
          app.shutdown();

        ImGui::EndMainMenuBar();
      }

      if (show_game_info) {
        ImGui::Begin("Game Info", NULL, ImGuiWindowFlags_NoFocusOnAppearing);
        {
          for (int i = 0; i < entities_player.size(); i++) {
            GameObject2D& player = entities_player[i];
            ImGui::Text("PLAYER_ID: %i", player.id);
            ImGui::Text("PLAYER_HP_MAX %i", player.hits_able_to_be_taken);
            ImGui::Text("PLAYER_HITS_TAKEN %i", player.hits_taken);
            ImGui::Text("PLAYER_BOOST %f", player.shift_boost_time_left);

#ifdef _DEBUG
            ImGui::Text("pos %f %f", player.pos.x, player.pos.y);
            ImGui::Text("vel x: %f y: %f", player.velocity.x, player.velocity.y);
            ImGui::Text("angle %f", player.angle_radians);
#endif
            ImGui::Separator();
          }

#ifdef _DEBUG
          ImGui::Text("game running for: %f", app.seconds_since_launch);
          ImGui::Text("camera pos %f %f", camera.pos.x, camera.pos.y);
          ImGui::Text("mouse pos %f %f", app.get_input().get_mouse_pos().x, app.get_input().get_mouse_pos().y);
          ImGui::Text("mouse grid pos %i %i", mouse_grid.x, mouse_grid.y);
          ImGui::Text("PhysicsGridSize %i", PHYSICS_GRID_SIZE);
          ImGui::Separator();
#endif

          // ImGui::Text("highscore: %i", highscore);
          ImGui::Text("(game) destroyed: %i", objects_destroyed);
          ImGui::Text("(game) enemy spawn rate: %f", wall_seconds_between_spawning_current);
#ifdef _DEBUG
          auto state_name = magic_enum::enum_name(state);
          ImGui::Text("(game) state: %s", std::string(state_name).data());

          // collect number of ai units in game
          {
            ai_behaviour behaviour = ai_behaviour::MOVEMENT_DIRECT;
            auto direct_ai =
              std::count_if(entities_enemies.begin(), entities_enemies.end(), [&behaviour](const GameObject2D& obj) {
                return obj.ai_current == behaviour;
              });
            ImGui::Text("(game) direct ai: %i", direct_ai);

            behaviour = ai_behaviour::MOVEMENT_ARC_ANGLE;
            auto arc_ai = std::count_if(entities_enemies.begin(),
                                        entities_enemies.end(),
                                        [&behaviour](const GameObject2D& obj) { return obj.ai_current == behaviour; });
            ImGui::Text("(game) arc ai %i", arc_ai);
          }

          bool temp = false;
          { // toggle shoot
            temp = game_player_shoot;
            ImGui::Checkbox("Player Shoot", &temp);
            if (temp != game_player_shoot) {
              std::cout << "player_shoot toggled to: " << temp << std::endl;
            }
            game_player_shoot = temp;
          }

          // collect number of ARC_ANGLE ai

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

      if (game_ui_show_inventory) {
        ImGui::Begin("Inventory", NULL, ImGuiWindowFlags_NoFocusOnAppearing);
        {
          ImGui::Text("A kangaroo");
          ImGui::Text("A baseball bat");
        }
        ImGui::End();
      }

      if (show_entity_menu) {

        std::vector<std::reference_wrapper<GameObject2D>> enemy;
        enemy.insert(enemy.end(), entities_enemies.begin(), entities_enemies.end());
        enemy.insert(enemy.end(), entities_bullets.begin(), entities_bullets.end());
        enemy.insert(enemy.end(), entities_player.begin(), entities_player.end());

        ImGui::Begin("Entity Menu", NULL, ImGuiWindowFlags_NoFocusOnAppearing);

        ImGui::Text("Players: %i", entities_player.size());
        ImGui::Text("Bullets: %i", entities_bullets.size());
        ImGui::Text("Enemies: %i", entities_enemies.size());

        ImGui::Separator();

        for (auto& e : enemy) {
          ImGui::Text("E: %s x:%i y:%i", e.get().name.c_str(), e.get().grid_position.x, e.get().grid_position.y);
        }

        ImGui::End();
      }

      if (show_game_console)
        console.Draw("Console", &show_game_console);
      if (show_profiler)
        profiler_panel::draw(profiler, delta_time_s);
      if (show_imgui_demo_window)
        ImGui::ShowDemoWindow(&show_imgui_demo_window);
    }
    profiler.end(Profiler::Stage::GuiLoop);
    profiler.begin(Profiler::Stage::FrameEnd);
    {
      debug_advance_one_frame = false;
      app.frame_end(frame_start_time);
    }
    profiler.end(Profiler::Stage::FrameEnd);
    profiler.end(Profiler::Stage::UpdateLoop);
  }
}
