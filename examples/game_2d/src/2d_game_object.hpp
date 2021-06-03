#pragma once

// your project headers
#include "spritemap.hpp"

// other project headers
#include <SDL2/SDL_scancode.h>
#include <glm/glm.hpp>

namespace game2d {

enum class CollisionLayer
{
  Default = 0,
  Player = 1,
  Bullet = 2,
  Destroyable = 3,

  Count = 4
};

struct KeysAndState
{
  int associated_player_id = 0;

  SDL_Scancode w = SDL_SCANCODE_W;
  SDL_Scancode s = SDL_SCANCODE_S;
  SDL_Scancode a = SDL_SCANCODE_A;
  SDL_Scancode d = SDL_SCANCODE_D;
  SDL_Scancode key_boost = SDL_SCANCODE_LSHIFT;
  SDL_Scancode key_pause = SDL_SCANCODE_P;

  SDL_Scancode key_camera_up = SDL_SCANCODE_UP;
  SDL_Scancode key_camera_down = SDL_SCANCODE_DOWN;
  SDL_Scancode key_camera_left = SDL_SCANCODE_LEFT;
  SDL_Scancode key_camera_right = SDL_SCANCODE_RIGHT;
  SDL_Scancode key_camera_follow_player = SDL_SCANCODE_Q;

  bool use_keyboard = false;
  float l_analogue_x = 0.0f;
  float l_analogue_y = 0.0f;
  float r_analogue_x = 0.0f;
  float r_analogue_y = 0.0f;
  bool pause_pressed = false;
  bool shoot_pressed = false;
  bool boost_pressed = false;
};

struct GameObject2D
{
  static inline uint32_t global_int_counter = 0;
  uint32_t id = 0;

  bool do_render = true;
  bool do_lifecycle_timed = false;
  bool do_lifecycle_health = true;
  bool do_physics = true;
  bool flag_for_delete = false;

  // render
  int tex_slot = 0;
  sprite::type sprite = sprite::type::SQUARE;
  glm::vec2 pos = { 0.0f, 0.0f }; // in pixels, centered
  glm::vec2 size = { 20.0f, 20.0f };
  float angle_radians = 0.0f;
  glm::vec4 colour = { 1.0f, 0.0f, 0.0f, 1.0f };

  // game: movement
  float speed_current = 50.0f;
  float speed_default = 50.0f;
  glm::vec2 velocity = { 0.0f, 0.0f };
  float velocity_boost_modifier = 2.0f;
  float shift_boost_time = 5.0f;
  float shift_boost_time_left = shift_boost_time;

  // game: shooting
  float bullet_seconds_between_spawning = 0.15f;
  float bullet_seconds_between_spawning_left = 0.0f;

  // game: lifecycle timed
  float time_alive_left = 5.0f;

  // game: lifecycle health
  int hits_able_to_be_taken = 3;
  int hits_taken = 0;
  int other_objects_destroyed = 0;
  bool invulnerable = false;

  // enemy ai: target
  // int player_id = 0;

  // game: extra
  std::string name = "DEFAULT";

  // physics
  CollisionLayer collision_layer = CollisionLayer::Default;

  GameObject2D() { id = ++GameObject2D::global_int_counter; }
};

[[nodiscard]] glm::vec2
gameobject_in_worldspace(const GameObject2D& camera, const GameObject2D& go);

[[nodiscard]] bool
gameobject_off_screen(glm::vec2 pos, glm::vec2 size, const glm::ivec2& screen_size);

} // namespace game2d
